#ifndef PARSER_H
#define PARSER_H

#include <vector>

#include "ast.h"
#include "line.h"

std::vector<BlockPtr> parse_blocks(const std::vector<LineInfo>& lines);

#endif  // PARSER_H
