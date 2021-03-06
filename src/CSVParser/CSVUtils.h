/**
 * @file        CSVUtils.h
 * @author      Argishti Ayvazyan (ayvazyan.argishti@gmail.com)
 * @brief       Declaration the utilities for Parser.
 * @date        31-10-2020
 * @copyright   Copyright (c) 2020
 */

#pragma once

#include <vector>

namespace csv
{
class Parser;
} // namespace csv


namespace csv::util
{

template <typename T>
using Table = std::vector<std::vector<T>>;

/**
 * @brief The execution policy type.
 */
enum class Execution
{
    /**
     * @brief Execution policy type used to indicate that an algorithm’s execution not parallelized.
     */
    Seq,

    /**
     * @brief Execution policy type used to indicate that an algorithm’s execution may be parallelized.
     */
    Par,

    /**
     * @brief Execution policy type used to indicate that an algorithm’s execution parallelized.
     */
    Par2
};

/**
 * @brief   Helper function for quickly reading CSV files that contain data of the same type.
 *
 * @tparam  T The CSV file data type.
 * @param   parser The source parser.
 * @param   exec The execution policy.
 * @return  The table contains the CSV file data.
 */
template <typename T>
Table<T> loadFlatCSV(const csv::Parser& parser, csv::util::Execution exec);


extern template Table<short> loadFlatCSV<short>(const csv::Parser&, const Execution);
extern template Table<unsigned short> loadFlatCSV<unsigned short>(const csv::Parser&, const Execution);
extern template Table<int> loadFlatCSV<int>(const csv::Parser&, const Execution);
extern template Table<unsigned> loadFlatCSV<unsigned>(const csv::Parser&, const Execution);
extern template Table<long> loadFlatCSV<long>(const csv::Parser&, const Execution);
extern template Table<unsigned long> loadFlatCSV<unsigned long>(const csv::Parser&, const Execution);
extern template Table<long long> loadFlatCSV<long long>(const csv::Parser&, const Execution);
extern template Table<unsigned long long> loadFlatCSV<unsigned long long>(const csv::Parser&, const Execution);
extern template Table<float> loadFlatCSV<float>(const csv::Parser&, const Execution);
extern template Table<double> loadFlatCSV<double>(const csv::Parser&, const Execution);
extern template Table<long double> loadFlatCSV<long double>(const csv::Parser&, const Execution);
extern template Table<std::string> loadFlatCSV<std::string>(const csv::Parser&, const Execution);
extern template Table<std::string_view> loadFlatCSV<std::string_view>(const csv::Parser&, const Execution);


/**
 * @brief               Generates the random CSV Table.
 *
 * @tparam T            The cell type.
 * @param rowCount      The count of row.
 * @param columnCount   The count of column.
 * @return              The generated table.
 */
template <typename T>
Table<T> generateRandomTable(size_t rowCount, size_t columnCount);

extern template Table<short> generateRandomTable<short>(size_t, size_t);
extern template Table<unsigned short> generateRandomTable<unsigned short>(size_t, size_t);
extern template Table<int> generateRandomTable<int>(size_t, size_t);
extern template Table<unsigned> generateRandomTable<unsigned>(size_t, size_t);
extern template Table<long> generateRandomTable<long>(size_t, size_t);
extern template Table<unsigned long> generateRandomTable<unsigned long>(size_t, size_t);
extern template Table<long long> generateRandomTable<long long>(size_t, size_t);
extern template Table<unsigned long long> generateRandomTable<unsigned long long>(size_t, size_t);
extern template Table<float> generateRandomTable<float>(size_t, size_t);
extern template Table<double> generateRandomTable<double>(size_t, size_t);
extern template Table<long double> generateRandomTable<long double>(size_t, size_t);

} // namespace csv::util
