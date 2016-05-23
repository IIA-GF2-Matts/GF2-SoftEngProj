
#include "scanner.h"
#include "gtest/gtest.h"

#include <string>
#include <sstream>
#include <ostream>
#include "errorhandler.h"
#include "names.h"
#include "network.h"
#include "devices.h"


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

std::map<TokType, const char*> _tokMap = {
    {EndOfFile, "EndOfFile"},
    {DevKeyword, "Device"},
    {MonitorKeyword, "Monitors"},
    {AsKeyword, "As"},
    {Equals, "Equals"},
    {Colon, "Colon"},
    {SemiColon, "SemiColon"},
    {Comma, "Comma"},
    {Brace, "Brace"},
    {CloseBrace, "CloseBrace"},
    {Dot, "Dot"},
    {Number, "Number"},
    {Identifier, "Identifier"},
    {DeviceType, "Type"}
};


class ScannerTest : public ::testing::Test {
    protected:

    names* nmz;
    devices *dmz;
    network *netz;

    virtual void SetUp() {
        nmz = new names();
        netz = new network(nmz);
        dmz = new devices(nmz, netz);
    }

    virtual void TearDown() {

    }


    const char* TokTypeText(TokType tp) {
        return _tokMap[tp];
    }

    void openAndloopThroughTokenStream(std::string s){
        std::istringstream iss(s);
        scanner scan(nmz);
        scan.open(&iss, "");

        while (scan.step().type != EndOfFile) {
        }
    }

    void testscannerError(std::string s) {
        EXPECT_THROW(openAndloopThroughTokenStream(s), matterror);
    }

    void testscannerToken(std::string s, TokType t) {

        std::istringstream iss(s);
        scanner scan(nmz);
        scan.open(&iss, "");


        Token tk = scan.step();

        EXPECT_EQ(t, tk.type);
        EXPECT_EQ(EndOfFile, scan.step().type);
    }

    void testscannerTokenStream(std::string s, std::vector<Token> stream) {
        strscanner scan(nmz, s);

        int tn = 0;
        Token tk;
        do {
            tk = scan.step();

            EXPECT_EQ(stream[tn].type, tk.type) << "Token type different at position " << tn
                << ", expected " << TokTypeText(stream[tn].type)
                << " got " << TokTypeText(tk.type);

            if (tk.type == Identifier) {
                EXPECT_EQ(stream[tn].id, tk.id) << "Itentifier name different at position " << tn
                << ", expected " << *stream[tn].id
                << " got " << *tk.id;
            }
            else if (tk.type == Number) {
                EXPECT_EQ(stream[tn].number, tk.number) << "Itentifier number different at position " << tn
                << ", expected " << stream[tn].number
                << " got " << tk.number;
            }

            tn++;
        } while (tk.type != EndOfFile);
    }

    Token genToken(TokType tktype) {
        return Token(tktype);
    }
    Token genToken(TokType tktype, int num) {
        return Token(tktype, num);
    }
    Token genToken(TokType tktype, namestring str) {
        if (tktype == DeviceType) 
            return Token(DeviceType, dmz->devkind(nmz->lookup(str)));
        else
            return Token(tktype, nmz->lookup(str));
    }
};

TEST_F(ScannerTest, PunctuationToken){
    testscannerToken("=", Equals);
    testscannerToken(":", Colon);
    testscannerToken(",", Comma);
    testscannerToken(";", SemiColon);
    testscannerToken("{", Brace);
    testscannerToken("}", CloseBrace);
    testscannerToken(".", Dot);
}

TEST_F(ScannerTest, DeviceTypeToken){
    testscannerToken("CLOCK",  DeviceType);
    testscannerToken("SWITCH", DeviceType);
    testscannerToken("AND",    DeviceType);
    testscannerToken("NAND",   DeviceType);
    testscannerToken("OR",     DeviceType);
    testscannerToken("NOR",    DeviceType);
    testscannerToken("DTYPE",  DeviceType);
    testscannerToken("XOR",    DeviceType);
}

TEST_F(ScannerTest, CaseInsensitiveToken){
    testscannerToken("ClOcK",   DeviceType);
    testscannerToken("dEv",     DevKeyword);
    testscannerToken("monItOR", MonitorKeyword);
    testscannerToken("nand",    DeviceType);
    testscannerToken("DEV",     DevKeyword);
}


TEST_F(ScannerTest, KeywordToken){
    testscannerToken("dev",     DevKeyword);
    testscannerToken("monitor", MonitorKeyword);
    testscannerToken("as",      AsKeyword);
}

TEST_F(ScannerTest, IdentifierToken){
    testscannerToken("G1",    Identifier);
    testscannerToken("SW1",   Identifier);
    testscannerToken("hello", Identifier);
}

TEST_F(ScannerTest, IdentifierTokenUnderscore){
    testscannerToken("G_",    Identifier);
    testscannerToken("S_W1",   Identifier);
    testscannerToken("h1_", Identifier);
}

TEST_F(ScannerTest, NumberToken){
    testscannerToken("1",   Number);
    testscannerToken("123", Number);
    testscannerToken("000", Number);
}

TEST_F(ScannerTest, LineCommentToken){
    testscannerToken("// Testing dev\n// Another comment?\n123", Number);
    testscannerToken("// Testing clock\n123// Another comment?",   Number);
    testscannerToken("//*/dev\nas",   AsKeyword);
}

TEST_F(ScannerTest, BlockCommentToken){
    testscannerToken("/* dev\n// Another comment?\nCLOCK*/dev", DevKeyword);
    testscannerToken("/*** dev\n// Another comment?\nCLOCK***/dev", DevKeyword);
    testscannerToken("/* *///dev\n123", Number);
    testscannerToken("/**/123", Number);
}






// TOKEN STREAMS



// device definitions

TEST_F(ScannerTest, DeviceDefinitionTokenStream){
    testscannerTokenStream("dev D1 = DTYPE { I1 : 0; }", {
        genToken(DevKeyword),
        genToken(Identifier, "D1"),
        genToken(Equals),
        genToken(DeviceType, "DTYPE"),
        genToken(Brace),
        genToken(Identifier, "I1"),
        genToken(Colon),
        genToken(Number, 0),
        genToken(SemiColon),
        genToken(CloseBrace),
        genToken(EndOfFile)
    });
}

TEST_F(ScannerTest, DeviceTypeDefinitionTokenStream){
    testscannerTokenStream("dev D1 = DTYPE;", {
        genToken(DevKeyword),
        genToken(Identifier, "D1"),
        genToken(Equals),
        genToken(DeviceType, "DTYPE"),
        genToken(SemiColon),
        genToken(EndOfFile)
    });
}

TEST_F(ScannerTest, DeviceOptionsDefinitionTokenStream){
    testscannerTokenStream("dev D1 {key:value}", {
        genToken(DevKeyword),
        genToken(Identifier, "D1"),
        genToken(Brace),
        genToken(Identifier, "key"),
        genToken(Colon),
        genToken(Identifier, "value"),
        genToken(CloseBrace),
        genToken(EndOfFile)
    });
}

TEST_F(ScannerTest, PreceedingKeywordIdentifiersTokenStream){
    testscannerTokenStream("dev devdevdevid dev;", {
        genToken(DevKeyword),
        genToken(Identifier, "devdevdevid"),
        genToken(DevKeyword),
        genToken(SemiColon),
        genToken(EndOfFile)
    });
}

TEST_F(ScannerTest, IdentifiersDotTokenStream){
    testscannerTokenStream("G1.I1", {
        genToken(Identifier, "G1"),
        genToken(Dot),
        genToken(Identifier, "I1"),
        genToken(EndOfFile)
    });

    testscannerTokenStream("dev_.I1", {
        genToken(Identifier, "dev_"),
        genToken(Dot),
        genToken(Identifier, "I1"),
        genToken(EndOfFile)
    });
}


TEST_F(ScannerTest, LineCommentNewlinesTokenStream){
    testscannerTokenStream("monitor//. as //asdf \r\n 3", {
        genToken(MonitorKeyword),
        genToken(Number, 3),
        genToken(EndOfFile)
    });

    testscannerTokenStream("monitor//. as //asdf \n 3", {
        genToken(MonitorKeyword),
        genToken(Number, 3),
        genToken(EndOfFile)
    });
}

TEST_F(ScannerTest, NestedLineCommentTokenStream){
    testscannerTokenStream("monitor//. as //asdf \n 3", {
        genToken(MonitorKeyword),
        genToken(Number, 3),
        genToken(EndOfFile)
    });


    testscannerTokenStream("monitor///*. as /*/asdf \n 3", {
        genToken(MonitorKeyword),
        genToken(Number, 3),
        genToken(EndOfFile)
    });


    testscannerTokenStream("monitor//*. as /*/asdf \n 3", {
        genToken(MonitorKeyword),
        genToken(Number, 3),
        genToken(EndOfFile)
    });

}

TEST_F(ScannerTest, BlockCommentTokenStream){
    testscannerTokenStream("sadf/**/as", {
        genToken(Identifier, "sadf"),
        genToken(AsKeyword),
        genToken(EndOfFile)
    });

    testscannerTokenStream("sadf/*\n*/as", {
        genToken(Identifier, "sadf"),
        genToken(AsKeyword),
        genToken(EndOfFile)
    });


    testscannerTokenStream("sadf/*//*/as", {
        genToken(Identifier, "sadf"),
        genToken(AsKeyword),
        genToken(EndOfFile)
    });


    testscannerTokenStream("sadf/*/* /*/as", {
        genToken(Identifier, "sadf"),
        genToken(AsKeyword),
        genToken(EndOfFile)
    });



}

TEST_F(ScannerTest, NumberTokenStream){
    testscannerTokenStream("1234.33", {
        genToken(Number, 1234),
        genToken(Dot),
        genToken(Number, 33),
        genToken(EndOfFile)
    });


    testscannerTokenStream("01", {
        genToken(Number, 1),
        genToken(EndOfFile)
    });
}

// ERRORS

TEST_F(ScannerTest, ErrorUnderscore){
    testscannerError("_G_");
    testscannerError("_dev");
    testscannerError("_01");
}

TEST_F(ScannerTest, ErrorIllegalCharacters){
    testscannerError("@");
    testscannerError("#");
    testscannerError("[");
    testscannerError("]");
    testscannerError("<");
    testscannerError("*");
    testscannerError("(");
    testscannerError("-");
}

TEST_F(ScannerTest, NestedBlockToken){
    testscannerError("asdf /*/* */*/as");
}

TEST_F(ScannerTest, BigNumberToken){
    testscannerError("1234567891234567");
}


TEST_F(ScannerTest, NegativeNumberToken){
    testscannerError("-1234");
}
