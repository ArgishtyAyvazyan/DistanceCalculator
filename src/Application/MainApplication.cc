/**
 * @file        MainApplication.cc
 * @author      Argishti Ayvazyan (ayvazyan.argishti@gmail.com)
 * @brief       Implementation for MainApplication.
 * @date        07-11-2020
 * @copyright   Copyright (c) 2020
 */

#include "MainApplication.h"

#include <sstream>
#include <fstream>
#include <map>
#include <memory>
#include <future>

#include "CLIParser.h"
#include "Msg.h"

#include "Task.h"

#include "CSVParser.h"
#include "CSVUtils.h"

#include "MathKernel.h"
#include "MTMathKernel.h"

namespace
{
    using namespace std::string_literals;
    using namespace std::string_view_literals;

    auto metricToEnum(std::string_view metric) -> math::EDistanceMetric
    {
        static const std::map<std::string_view, math::EDistanceMetric> s_mapNameToEnum
                {
                        { "L1", math::EDistanceMetric::L1 },
                        { "L2", math::EDistanceMetric::L2 },
                        { "Hamming", math::EDistanceMetric::Hamming },
                };

        const auto search = s_mapNameToEnum.find(metric);
        ASSERT_ERROR(search != s_mapNameToEnum.end(),
                     ("Invalid math metric name: "s + metric.data()).c_str());

        return search->second;
    }

} // unnamed namespace


namespace dc
{

constexpr auto* s_ApplicationDescription = "The distances calculator tool,"
                                           " designed to calculate distances between big numerical vectors.";

MainApplication::MainApplication(int argc, char** argv) noexcept
    : m_argc(argc),
    m_argv(argv),
    m_dbg(false),
    m_executeParallel(false),
    m_pMPIWrapper(std::make_unique<mpi::MPIWrapper>(&argc, &argv))
{
    if (!m_pMPIWrapper->isMain())
    {
        io::Msg::DisableMessages(true);
    }
}

int MainApplication::run()
{
    try
    {
        START_TASK("Distance calculator");

        if (!parseAndInitParameters(m_argc, m_argv))
        {
            return 0; // Called help.
        }

        checkArguments();
        showSummary();

        csv::util::Table<TValueType> dataSet;
        if (m_pMPIWrapper->isMain())
        {
            auto[loadedQuerySet, loadedDataSet] = loadCSVFiles();
            m_pMPIWrapper->distributeTask(loadedQuerySet, loadedDataSet);
            dataSet = std::move(loadedDataSet);
        }

        auto querySet = m_pMPIWrapper->receiveQuery();
        if (!m_pMPIWrapper->isMain())
        {
            dataSet = m_pMPIWrapper->receiveDataSet();
        }

        auto distancesMatrix = computeDistances(querySet, dataSet);

        m_pMPIWrapper->sendDistanceMatrix(distancesMatrix);

        if (m_pMPIWrapper->isMain())
        {
            auto fullDistancesMatrix = m_pMPIWrapper->receiveDistanceMatrix();
            if (m_outFile.empty())
            {
                displayResult(fullDistancesMatrix);
            } else
            {
                writeCSV(fullDistancesMatrix);
            }
        }

        io::Msg::Write("The distance computing completed successfully.");
    }
    catch (const dbgh::CAssertException& e)
    {
        uncoverException(e);
        io::Msg::Write(e.Message(), io::MsgType::Error);
    }
    return 0;
}

bool MainApplication::parseAndInitParameters(int argc, char** argv)
{
    io::cli::Parser parser { argc, argv, s_ApplicationDescription };
    std::string queryFilePath;
    parser.addOption("-query", queryFilePath, ""
                     , "The path to the CSV file, that contains query vectors.");

    std::string dataSetFilePath;
    parser.addOption("-dataSet", dataSetFilePath, ""
                     , "The path to the CSV file, that contains dataSet vectors.");

    std::string outFilePath;
    parser.addOption("-out", outFilePath, ""
                     , "The output file directory by default current dir.");

    parser.addOption("-parallel", m_executeParallel, false
                 , "Execute the parallel.");

    parser.addOption("-metric", m_strMetric, "L1", "The math metric type.");

    parser.addOption("-dbg", m_dbg, false
                     , "Run application in debug mode.");

    parser.addOption("-dumpTimeLog", []()
    {
        task::TaskManager::get().enableLogging(true);
    }, false, "Dump the tasks execution times into the time.log file.");

    const auto parsedHelp = !parser.parse();
    if (parsedHelp)
    {
        return false;
    }

    auto initIfNotEmpty = [](std::filesystem::path& path, const std::string& strPath) {
        if (!strPath.empty())
        {
            path = std::filesystem::absolute(strPath).lexically_normal();
        }
    };

    initIfNotEmpty(m_queryCSVFile, queryFilePath);
    initIfNotEmpty(m_dataSetCSVFile, dataSetFilePath);
    initIfNotEmpty(m_outFile, outFilePath);

    return true;
}

void MainApplication::checkArguments() const
{
    io::Msg::Write("Analyze parameters."sv);

    ASSERT_ERROR(m_queryCSVFile.empty() || std::filesystem::exists(m_queryCSVFile)
        ,("The query file not exists. Path: "s + m_queryCSVFile.string()).c_str());

    ASSERT_ERROR(m_dataSetCSVFile.empty() || std::filesystem::exists(m_dataSetCSVFile)
        ,("The data set file not exists. Path: "s + m_dataSetCSVFile.string()).c_str());

    ASSERT_ERROR((m_queryCSVFile.empty() && m_dataSetCSVFile.empty())
                || (!m_queryCSVFile.empty() && !m_dataSetCSVFile.empty()),
                "Only one set cannot generate automatically.");

    ASSERT_ERROR(m_outFile.empty() || !std::filesystem::exists(m_outFile)
                 , ("The output file already exists. File path: "s + m_outFile.string()).c_str());

    if (m_queryCSVFile == m_dataSetCSVFile)
    {
        io::Msg::Write("The query and data set paths is equal.", io::MsgType::Warning);
    }
}

void MainApplication::showSummary() const
{
    io::Msg::Write("The execution summary:"sv);

    if (m_dbg)
    {
        io::Msg::Write("Running in debug mode.", io::MsgType::Warning);
    }

    auto pathOrMassage = [](const std::filesystem::path& path) {
        return (!path.empty())
            ? path.string()
            : "File path is empty, the set will be generated randomly."s;
    };

    std::stringstream ss;
    ss << std::endl;
    ss << "The query path:                      " << pathOrMassage(m_queryCSVFile) << std::endl;
    ss << "The data set path:                   " << pathOrMassage(m_dataSetCSVFile) << std::endl;
    ss << "The output path:                     " << m_outFile << std::endl;
    ss << "The math metric type:                " << m_strMetric << std::endl;
    ss << "Execute parallel:                    " << std::boolalpha << m_executeParallel << std::endl;
    ss << "Running in the multi-process flow:   " << m_pMPIWrapper->isMPF() << std::endl;
    ss << "The available processes:             " << m_pMPIWrapper->numOfProcessor() << std::endl;
    io::Msg::Write(ss.str().c_str());
}

auto MainApplication::createDistanceCalculator() const -> math::DistanceCalculator<TValueType>
{
    std::unique_ptr<math::IMathKernel<TValueType>> kernel;

    if (m_executeParallel)
    {
        kernel = std::make_unique<math::MTMathKernel<TValueType>>();
    }
    else
    {
        kernel = std::make_unique<math::MathKernel<TValueType>>();
    }

    return math::DistanceCalculator<TValueType>(std::move(kernel));
}

auto MainApplication::loadCSVFiles() const -> std::pair<csv::util::Table<TValueType>, csv::util::Table<TValueType>>
{
    START_TASK("CSV files loading");

    auto load = [this](const std::string& path) -> csv::util::Table<TValueType>
    {
        if (path.empty())
        {
            return csv::util::generateRandomTable<TValueType>(1024, 1024);
        }

        csv::Parser Parser { path };
        const auto policy = m_executeParallel ? csv::util::Execution::Par
                : csv::util::Execution::Seq;
        return csv::util::loadFlatCSV<TValueType>(Parser, policy);
    };

    if (!m_executeParallel)
    {
        // Sequential reading.
        return { load(m_queryCSVFile.string()), load(m_dataSetCSVFile.string()) };
    }

    // Parallel reading.
    auto loadQueryParser = std::async(std::launch::async, load, m_queryCSVFile.string());
    auto loadDataSetParser = std::async(std::launch::async, load, m_dataSetCSVFile.string());

    std::string errorMessage;

    auto futureResult = [&errorMessage, this](auto& table, auto& future)
    {
        try
        {
            table = future.get();
        }
        catch (const dbgh::CAssertException& e)
        {
            uncoverException(e);
            if (!errorMessage.empty())
            {
                errorMessage += " | ";
            }
            errorMessage += e.Message();
        }
    };

    csv::util::Table<TValueType> querySet;
    futureResult(querySet, loadQueryParser);

    csv::util::Table<TValueType> dataSet;
    futureResult(dataSet, loadDataSetParser);

    ASSERT_ERROR(errorMessage.empty(), errorMessage.c_str());

    return { querySet, dataSet };
}

auto MainApplication::computeDistances(
        const csv::util::Table<MainApplication::TValueType>& query,
        const csv::util::Table<MainApplication::TValueType>& dataSet
        ) -> csv::util::Table<MainApplication::TValueType>
{
    START_TASK("Compute distances");
    auto distanceCalculator = createDistanceCalculator();
    const auto metric = metricToEnum(m_strMetric);
    return distanceCalculator.computeDistance(query, dataSet, metric);
}

void MainApplication::displayResult(const csv::util::Table<MainApplication::TValueType>& table)
{
    std::stringstream ss;
    ss << "The distance matrix:" << std::endl;
    ss << std::endl;
    for (const auto& row : table)
    {
        for (const auto cell : row)
        {
            ss << cell << ", ";
        }
        ss << std::endl;
    }

    io::Msg::Write(ss.str());
}

void MainApplication::writeCSV(const csv::util::Table<MainApplication::TValueType>& table)
{
    START_TASK("Write distance matrix in out file");
    ASSERT_ERROR(!m_outFile.empty(), "The output file path is empty.");
    io::Msg::Write("The output file: "s + m_outFile.string());

    std::fstream fOutCSV { m_outFile, std::ios_base::out };
    for (const auto& row : table)
    {
        for (const auto cell : row)
        {
            fOutCSV << cell << ", ";
        }
        fOutCSV << std::endl;
    }
}

void MainApplication::uncoverException(const dbgh::CAssertException& e) const
{
    if (!m_dbg)
    {
        return;
    }
    std::stringstream ss;
    ss << std::endl;
    ss << "  [file]:         " << e.FileName() << std::endl;
    ss << "  [line]:         " << e.LineNumber() << std::endl;
    ss << "  [function]:     " << e.Function() << std::endl;
    ss << "  [expression]:   " << e.Expression() << std::endl;
    ss << "  [what]:         " << e.Message() << std::endl;
    ss << std::endl;
    io::Msg::Write(ss.str(), io::MsgType::Error);
}

} // namespace dc
