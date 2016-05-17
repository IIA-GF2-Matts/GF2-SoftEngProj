
#include "scanner.h"
#include "gtest/gtest.h"

#include <string>
#include <sstream>
#include "errorhandler.h"

/*


    EndOfFile,
    DevKeyword,
    MonitorKeyword,
    AsKeyword,
    Equals,
    Colon,
    SemiColon,
    Comma,
    Brace,
    CloseBrace,
    Dot,
    Number,
    Identifier,
    DeviceType

    */

class ScannerTest : public ::testing::Test {
    protected:
        virtual void SetUp() {
    }

    virtual void TearDown() {

    }

    void testscannerTokenError(std::string s) {
        std::istringstream iss(s);

        scanner scan;

        EXPECT_THROW(scan.open(&iss, "");, matterror);
    }

    void testscannerToken(std::string s, TokType t) {

        std::istringstream iss(s);

        scanner scan;
        scan.open(&iss, "");


        Token tk = scan.step();

        EXPECT_EQ(t, tk.type);
        EXPECT_EQ(TokType::EndOfFile, scan.step().type);
    }
};

TEST_F(ScannerTest, PunctuationToken){
    testscannerToken("=", TokType::Equals);
    testscannerToken(":", TokType::Colon);
    testscannerToken(",", TokType::Comma);
    testscannerToken(";", TokType::SemiColon);
    testscannerToken("{", TokType::Brace);
    testscannerToken("}", TokType::CloseBrace);
    testscannerToken(".", TokType::Dot);
}

TEST_F(ScannerTest, DeviceTypeToken){
    testscannerToken("CLOCK",  TokType::DeviceType);
    testscannerToken("SWITCH", TokType::DeviceType);
    testscannerToken("AND",    TokType::DeviceType);
    testscannerToken("NAND",   TokType::DeviceType);
    testscannerToken("OR",     TokType::DeviceType);
    testscannerToken("NOR",    TokType::DeviceType);
    testscannerToken("DTYPE",  TokType::DeviceType);
    testscannerToken("XOR",    TokType::DeviceType);
}

TEST_F(ScannerTest, CaseInsensitiveToken){
    testscannerToken("ClOcK",   TokType::DeviceType);
    testscannerToken("dEv",     TokType::DevKeyword);
    testscannerToken("monItOR", TokType::MonitorKeyword);
    testscannerToken("nand",    TokType::DeviceType);
    testscannerToken("DEV",     TokType::DevKeyword);
}


TEST_F(ScannerTest, KeywordToken){
    testscannerToken("dev",     TokType::DevKeyword);
    testscannerToken("monitor", TokType::MonitorKeyword);
    testscannerToken("as",      TokType::AsKeyword);
}

TEST_F(ScannerTest, IdentifierToken){
    testscannerToken("G1",    TokType::Identifier);
    testscannerToken("SW1",   TokType::Identifier);
    testscannerToken("hello", TokType::Identifier);
}

TEST_F(ScannerTest, IdentifierTokenUnderscore){
    testscannerToken("G_",    TokType::Identifier);
    testscannerToken("S_W1",   TokType::Identifier);
    testscannerToken("h1_", TokType::Identifier);
}

TEST_F(ScannerTest, NumberToken){
    testscannerToken("1",   TokType::Number);
    testscannerToken("123", TokType::Number);
    testscannerToken("000", TokType::Number);
}

TEST_F(ScannerTest, LineCommentToken){
    testscannerToken("// Testing dev\n// Another comment?\n123", TokType::Number);
    testscannerToken("// Testing clock\n123// Another comment?",   TokType::Number);
    testscannerToken("//*/dev\nas",   TokType::AsKeyword);
}

TEST_F(ScannerTest, BlockCommentToken){
    testscannerToken("/* dev\n// Another comment?\nCLOCK*/dev", TokType::DevKeyword);
    testscannerToken("/*** dev\n// Another comment?\nCLOCK***/dev", TokType::DevKeyword);
    testscannerToken("/* *///dev\n123", TokType::Number);
    testscannerToken("/**/123", TokType::Number);
}

// ERRORS

TEST_F(ScannerTest, ErrorUnderscore){
    testscannerTokenError("_G_");
    testscannerTokenError("_dev");
    testscannerTokenError("_01");
}

TEST_F(ScannerTest, ErrorIllegalCharacters){
    testscannerTokenError("@");
    testscannerTokenError("#");
    testscannerTokenError("[");
    testscannerTokenError("]");
    testscannerTokenError("<");
    testscannerTokenError("*");
    testscannerTokenError("(");
}