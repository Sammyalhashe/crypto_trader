#include <nlohmann/json.hpp>

#include <fstream>

#include <gtest/gtest.h>

import fileutils_module;

using Json = nlohmann::json;

TEST(FileUtils, JsonFileTest)
{
    static const char *fileName         = "testFile.json";
    Json               testFileContents = "{\
            \"someKey\": 42,\
            \"someNestedObject\": {\
                \"someNestedKey\": 420\
            }\
        }"_json;

    std::ofstream outFileStream;
    outFileStream.open(fileName);

    if (outFileStream.is_open()) {
        outFileStream << testFileContents;
        // NOTE: This is interesting, and I think it's a unix thing,
        // but you need this '\n' or else x
        outFileStream << '\n';
        outFileStream.close();
    }

    using namespace fileutils_module;

    Json result;
    int  rc = readJsonFile(&result, fileName);

    EXPECT_EQ(0, rc);
    EXPECT_EQ(testFileContents, result);
    EXPECT_EQ(result["someKey"], 42);
    EXPECT_EQ(result["someNestedObject"]["someNestedKey"], 420);
}
