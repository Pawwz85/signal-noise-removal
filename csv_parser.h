#pragma once
#include <string>
#include <vector>
#include <algorithm>
#include <stdexcept>
#include <fstream>
#include "signals.h"

namespace CSV_READER {
	static size_t _count_separators(const std::string& csv_line, const char sep) {
		return std::count(csv_line.begin(), csv_line.end(), sep);
	}

	static size_t count_channels(const std::string& csv_line, const char sep) {
		return 1 + _count_separators(csv_line, sep);
	}

	static void tokenize(const std::string& csv_line, const char sep, std::vector<std::string> & tokens) {
		std::string current_chunk = "";
		tokens.clear();
		for (char c : csv_line) {

			if (c == sep) {
				// flush current chunk
				tokens.push_back(current_chunk);
				current_chunk = "";
			} else {
				current_chunk += c;
			}

		}

		if (current_chunk != "")
			tokens.push_back(current_chunk);
	}

	class MalformedCSVFileException : public std::exception {
		std::string msg;
	public:
		MalformedCSVFileException(const char* message) : msg(message) {}

		// Override the what() method to provide an error message.
		const char* what() const throw() {
			return msg.c_str();
		}
	};

	static std::vector<std::vector<std::string>> parse_columns(const std::vector<std::string>& csv_lines, const char sep) {
		
		if (csv_lines.empty())
			throw MalformedCSVFileException("CSV File can not be empty");

		size_t channel_count = count_channels(csv_lines[0], sep);

		std::vector<std::vector<std::string>> result;
		std::vector<std::string> tokens;

		for (size_t i = 0; i < channel_count; ++i)
			result.push_back(std::vector<std::string>());


		for (auto& line : csv_lines) {
			tokenize(line, sep, tokens);
			if(tokens.size() != channel_count) throw MalformedCSVFileException("Channels must be equal in length");

			for (size_t i = 0; i < channel_count; ++i)
				result[i].push_back(tokens[i]);
		}

		return result;
	}

	static Signal<time_t>* parse_signal_from_column(const std::vector<std::string>& csv_column, time_t probing_period) {
		// TODO: check if column consists of valid 'double' values
		double* arr = new double[csv_column.size()];

		try {
			for (size_t i = 0; i < csv_column.size(); ++i) {
				arr[i] = std::stod(csv_column[i]);
			}
		}
		catch (std::invalid_argument) {
			throw MalformedCSVFileException("Unexpected non numeric value");
		}

		Signal<time_t>* result = load_signal(csv_column.size(), probing_period, arr);
		
		delete[] arr;
		return result;
	}

	static std::vector<std::unique_ptr<Signal<time_t>>> load_multichannel_signals_from_csv(const std::string& file_name, const char sep, time_t probing_period) {
		std::vector<std::string> lines;
		std::ifstream csv_file;
		
		csv_file.open(file_name);

		for (std::string line; std::getline(csv_file, line); )
			lines.push_back(line);
		
		csv_file.close();

		auto columns = parse_columns(lines, sep);

		std::vector<std::unique_ptr<Signal<time_t>>> result;
		result.reserve(columns.size());

		for (auto& column : columns)
			result.push_back(std::unique_ptr<Signal<time_t>>(parse_signal_from_column(column, probing_period)));

		return result;
	}

	static void save_single_channel_signal_to_csv(const std::string& file_name, std::weak_ptr<Signal<time_t>> signal) {
		std::shared_ptr<Signal<time_t>> ptr;
		if ((!signal.expired()) && (ptr = signal.lock()) && ptr != nullptr) {
		
			std::ofstream csv_file;
			csv_file.open(file_name);
			
			for (auto p = ptr->cbegin(); p != ptr->cend(); ++p) {
				csv_file << p->value << "\n";
			}
		}
	}

}