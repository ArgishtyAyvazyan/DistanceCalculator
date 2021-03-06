/**
 * @file        Parser.h
 * @author      Argishti Ayvazyan (ayvazyan.argishti@gmail.com)
 * @brief       Declaration for Parser.
 * @date        30-10-2020
 * @copyright   Copyright (c) 2020
 */

#pragma once

#include <string>
#include <string_view>
#include <vector>
#include <memory>

#include <DBGHAssert.h>

#include "CSVRow.h"
#include "CSVCell.h"
#include "CSVUtils.h"

namespace csv
{

/**
 * @class Parser
 * @brief The class for providing CSV file reading.
 *
 * @details After object initialization loads all data of the CSV file in virtual memory.
 *          After that, it is possible to read data.
 *          The class is Movable but not copyable.
 */
class Parser
{
    using TRowVector = std::vector<Row>;
public:

    using iterator = TRowVector::iterator;
    using const_iterator = TRowVector::const_iterator;
    using reverse_iterator = TRowVector::reverse_iterator;
    using const_reverse_iterator = TRowVector::const_reverse_iterator;

    Parser() = delete;
    Parser(const Parser&) = delete;
    Parser& operator=(const Parser&) = delete;

    Parser(Parser&&) = default;
    Parser& operator=(Parser&&) = default;
    ~Parser() = default;

    /**
     * @brief Contracts and initialized Parser object.
     * @param strCSVFileName The CSV file name.
     */
    explicit Parser(std::string strCSVFileName);

    /**
     * @brief Returns the iterator to the memory representation of the first line of the CSV file.
     * @return The iterator.
     */
    [[nodiscard]] auto begin() const noexcept { return std::begin(m_arrData); }

    /**
     * @brief Returns the iterator to the memory representation of the past-the-end line of the CSV file.
     * @return The iterator.
     */
    [[nodiscard]] auto end() const noexcept { return std::end(m_arrData); }

    /**
     * @brief Returns the iterator to the memory representation of the first line of the CSV file.
     * @return The const_iterator.
     */
    [[nodiscard]] auto cbegin() const noexcept { return std::cbegin(m_arrData); }

    /**
     * @brief Returns the iterator to the memory representation of the past-the-end line of the CSV file.
     * @return The const_iterator.
     */
    [[nodiscard]] auto cend() const noexcept { return std::cend(m_arrData); }


    /**
     * @brief Returns the iterator to the memory representation of the first line of the CSV file.
     * @return The reverse_iterator.
     */
    [[nodiscard]] auto rbegin() const noexcept { return std::rbegin(m_arrData); }


    /**
     * @brief Returns the iterator to the memory representation of the past-the-end line of the CSV file.
     * @return The reverse_iterator.
     */
    [[nodiscard]] auto rend() const noexcept { return std::rend(m_arrData); }


    /**
     * @brief Returns the iterator to the memory representation of the first line of the CSV file.
     * @return The const_reverse_iterator.
     */
    [[nodiscard]] auto crbegin() const noexcept { return std::crbegin(m_arrData); }


    /**
     * @brief Returns the iterator to the memory representation of the past-the-end line of the CSV file.
     * @return The const_reverse_iterator.
     */
    [[nodiscard]] auto crend() const noexcept { return std::crend(m_arrData); }


    /**
     * @brief Returns the count of row in the CSV file.
     * @return std::size_t the number of rows.
     */
    std::size_t size() const noexcept;
private:

    /**
     * @internal
     * @brief Loads CSV file and store lines in \ref m_arrData.
     */
    void loadCSVFile();

private:

    /**
     * @internal
     * @brief The CSV file name.
     */
    std::string m_strCSVFileName;

    /**
     * @internal
     * @brief The vector contains lines of CSV file after loading.
     */
    TRowVector m_arrData;

}; // class Parser


} // namespace csv
