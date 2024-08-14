#include "lexer.h"

#include <string>

std::vector<std::string> split_lines(const std::string& text) {
    std::vector<std::string> res;
    size_t start = 0;
    while (true) {
        size_t pos = text.find('\n', start);
        if (pos == std::string::npos) {
            if (start < text.size()) {
                res.push_back(text.substr(start));
            }
            break;
        }
        size_t end = pos;
        if (end > start && text[end - 1] == '\r') {
            --end;
        }
        res.push_back(text.substr(start, end - start));
        start = pos + 1;
    }
    return res;
}

bool is_blank_text(const std::string& line) {
    for (size_t i = 0; i < line.size(); ++i) {
        if (line[i] != ' ' && line[i] != '\t') {
            return false;
        }
    }
    return true;
}

bool is_heading_text(const std::string& line, std::string& text, int& level) {
    size_t pos = 0;
    while (pos < line.size() && (line[pos] == ' ' || line[pos] == '\t')) {
        ++pos;
    }
    level = 0;
    while (pos < line.size() && line[pos] == '#' && level < 6) {
        ++level;
        ++pos;
    }
    if (level == 0 || pos >= line.size() || line[pos] != ' ') {
        return false;
    }
    ++pos;
    text = line.substr(pos);
    return true;
}

bool is_code_fence_text(const std::string& line, std::string& text) {
    if (line.size() < 3) {
        return false;
    }
    const std::string marker = line.substr(0, 3);
    if (marker != "```" && marker != "~~~") {
        return false;
    }
    text = line.substr(3);
    return true;
}

bool is_horizontal_rule_text(const std::string& line) {
    size_t pos = 0;
    while (pos < line.size() && line[pos] == ' ') {
        ++pos;
    }
    if (pos > 3 || pos >= line.size()) {
        return false;
    }
    const char signal = line[pos];
    if (signal != '-' && signal != '*' && signal != '_') {
        return false;
    }
    int cnt = 0;
    while (pos < line.size()) {
        if (line[pos] == signal) {
            ++cnt;
        } else if (line[pos] != ' ') {
            return false;
        }
        ++pos;
    }
    return cnt >= 3;
}

bool is_block_quote_text(const std::string& line, std::string& text) {
    size_t pos = 0;
    while (pos < line.size() && (line[pos] == ' ' || line[pos] == '\t')) {
        ++pos;
    }
    if (pos >= line.size() || line[pos] != '>') {
        return false;
    }
    ++pos;
    if (pos < line.size() && line[pos] == ' ') {
        ++pos;
    }
    text = line.substr(pos);
    return true;
}

bool is_unordered_list_text(const std::string& line, std::string& text) {
    if (line.size() < 2) {
        return false;
    }
    const char marker = line[0];
    if ((marker != '-' && marker != '*' && marker != '+') || line[1] != ' ') {
        return false;
    }
    text = line.substr(2);
    return true;
}

bool is_ordered_list_text(const std::string& line, std::string& text) {
    size_t pos = 0;
    if (pos >= line.size() || line[pos] < '0' || line[pos] > '9') {
        return false;
    }
    while (pos < line.size() && line[pos] >= '0' && line[pos] <= '9') {
        ++pos;
    }
    if (pos >= line.size() || line[pos] != '.') {
        return false;
    }
    ++pos;
    if (pos >= line.size() || line[pos] != ' ') {
        return false;
    }
    ++pos;
    text = line.substr(pos);
    return true;
}

LineInfo classify_line(const std::string& line) {
    LineInfo res;
    std::string content;
    int level = 0;
    if (is_blank_text(line)) {
        res = LineInfo(LineType::Blank, "", 0);
    } else if (is_heading_text(line, content, level)) {
        res = LineInfo(LineType::Heading, content, level);
    } else if (is_code_fence_text(line, content)) {
        res = LineInfo(LineType::CodeFence, content, 0);
    } else if (is_horizontal_rule_text(line)) {
        res = LineInfo(LineType::HorizontalRule, "", 0);
    } else if (is_block_quote_text(line, content)) {
        res = LineInfo(LineType::Blockquote, content, 0);
    } else if (is_unordered_list_text(line, content)) {
        res = LineInfo(LineType::UnorderedList, content, 0);
    } else if (is_ordered_list_text(line, content)) {
        res = LineInfo(LineType::OrderedList, content, 0);
    } else {
        res = LineInfo(LineType::Text, line, 0);
    }
    return res;
}

std::vector<LineInfo> tokenize(const std::string& text) {
    std::vector<LineInfo> lines;
    const std::vector<std::string> raw_lines = split_lines(text);
    lines.reserve(raw_lines.size());
    for (std::vector<std::string>::const_iterator it = raw_lines.begin();
         it != raw_lines.end(); ++it) {
        lines.push_back(classify_line(*it));
    }
    return lines;
}
