
#include <ostream>
#include <string>
#include <set>

#ifndef GF2_SOURCEPOS_H
#define GF2_SOURCEPOS_H


// Represents a position within a file.
struct SourcePos {
private:
    static std::set<std::string> fileNames;
    std::set<std::string>::const_iterator getFile(std::string f);

public:
    std::set<std::string>::const_iterator File;
	int Line;
	int Column;
	int Abs;

    const std::string& fileStr() const;
    void setFile(std::string f);

	SourcePos();
    SourcePos(int line, int col);
    SourcePos(int line, int col, int abs);
	SourcePos( std::string file, int line, int col, int abs);

	friend std::ostream& operator<<( std::ostream&, const SourcePos& );
};


#endif
