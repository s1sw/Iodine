/*#include "parser.hpp"
#include <string>
#include <fstream>
#include <vector>

using namespace iodine;

std::unordered_map<std::string, Variable> iodine::variables;
std::unordered_map<std::string, Function> iodine::functions;

int main(int argc, char** argv) {
    // get file name
    std::string fileName = argv[1];

    std::ifstream stream(fileName);

    std::vector<std::string> lines;

    while (stream.good()) {
        std::string line;
        std::getline(stream, line);

        std::shared_ptr<ASTNode> node = parseScript(parseTokens(line));

        if (!line.empty()) {
            lines.push_back(line);
        }

    }
} */
int main() { return 0; }
