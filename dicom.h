#ifndef DICOM_H
#define DICOM_H

#include <string>
#include <vector>

using namespace std;

struct DicomTag
{
    string group;
    string element;
};

class DicomData
{
public:
    DicomData(){};

    void set_tag(uint8_t tag[4]);
    void set_vr(char tag[2]);
    void set_reserved();
    void set_length(int length);

    string group() { return tag_.group; }
    string element() { return tag_.element; }
    string vr() { return vr_; }
    int length() { return length_; }
    vector<char> value() { return value_; }

private:
    DicomTag tag_;
    std::string vr_ = "";
    int length_ = 0;
    std::vector<char> value_;
};

class Dicom
{
public:
    Dicom(const char* file);
    vector<char> findValueByTag(string group, string element);
    void dump();
    void setImage(uint16_t value);
    vector<uint16_t> image() { return image_; }
    void setRows(uint16_t value) { row_ = (int) value; }
    void setColumns(uint16_t value) { column_ = (int) value; }
    int row() { return row_; }
    int column() { return column_; }

private:
    vector<DicomData> data_;
    vector<uint16_t> image_;
    int row_;
    int column_;
};

#endif // DICOM_H
