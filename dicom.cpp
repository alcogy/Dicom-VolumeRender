#include "dicom.h"
#include <iostream>
#include <fstream>
#include <cstddef>
#include <QDebug>

Dicom::Dicom(const char* filename)
{
    ifstream file(filename, ios::in | ios::binary);

    // Check Dicom file
    file.seekg(0x80);
    uint8_t prefix[4];
    file.read((char*)prefix, sizeof(char) * 4);
    if ((char)prefix[0] != 'D' ||
        (char)prefix[1] != 'I' ||
        (char)prefix[2] != 'C' ||
        (char)prefix[3] != 'M')
    {
        qDebug() << "Not Dicom file";
        return;
    }


    file.seekg(0x84);
    while(!file.eof())
    {
        DicomData data;
        uint8_t tag[4];
        file.read((char*)tag, sizeof(char) * 4);
        data.set_tag(tag);

        char vr[2];
        file.read(vr, sizeof(char) * 2);
        data.set_vr(vr);

        int value_length = 0;

        if (data.vr() == "OB"
            || data.vr() == "OW"
            || data.vr() == "OF"
            || data.vr() == "SQ"
            || data.vr() == "UT"
            || data.vr() == "UN")
        {
            char reserve[2];
            file.read(reserve, sizeof(char) * 2);
            uint32_t vl;
            file.read((char*) &vl, sizeof(uint32_t));

            if (vl == 0xffffffff)
            {
                while(true)
                {
                    uint8_t sq_tag[4];
                    file.read((char*)sq_tag, sizeof(char) * 4);
                    if (sq_tag[0] == 0xfe && sq_tag[1] == 0xff && sq_tag[3] == 0xe0)
                    {
                        if (sq_tag[2] == 0x00)
                        {
                            uint8_t data[4];
                            file.read((char*)data, sizeof(char) * 4);
                            continue;
                        }
                        if (sq_tag[2] == 0x0d)
                        {
                            uint8_t data[4];
                            file.read((char*)data, sizeof(char) * 4);
                            continue;
                        }
                        if (sq_tag[2] == 0xdd)
                        {
                            uint8_t data[4];
                            file.read((char*)data, sizeof(char) * 4);
                            break;
                        }
                    }
                    else
                    {
                        uint8_t sq_vr[2];
                        file.read((char*)sq_vr, sizeof(char) * 2);

                        uint16_t sq_vl;
                        file.read((char*) &sq_vl, sizeof(uint16_t));

                        int sq_length = (unsigned int) sq_vl;
                        char value[sq_length];
                        file.read(value, sq_length);
                    }
                }

            }
            else
            {
                value_length = (unsigned int) vl;
            }
        }
        else
        {
            uint16_t vl;
            file.read((char*) &vl, sizeof(uint16_t));
            value_length = (unsigned int) vl;
        }

        data.set_length(value_length);

        if (data.vr() != "SQ")
        {
            if (data.group() == "7fe0" && data.element() == "0010")
            {
                int size = value_length / 2;
                for (int i = 0; i < size; i++)
                {
                    uint16_t value;
                    file.read((char*) &value, sizeof(uint16_t));
                    setImage(value);
                }
            }
            else if (data.group() == "0028" && data.element() == "0010")
            {
                uint16_t value;
                file.read((char*) &value, sizeof(uint16_t));
                setRows(value);
            }
            else if (data.group() == "0028" && data.element() == "0011")
            {
                uint16_t value;
                file.read((char*) &value, sizeof(uint16_t));
                setColumns(value);
            }
            else
            {
                char value[value_length];
                file.read((char*) value, data.length());
            }

        }

        data_.push_back(data);

        if (data.group() == "7fe0" && data.element() == "0010") break;
    }

    file.close();

    // dump();
}

void Dicom::dump()
{
    for (auto data : data_)
    {
        cout << "Info: (" << data.group() << ", " << data.element() << ") " << data.vr() << endl;
    }
    cout << "[";
    for (auto image : image_)
    {
        cout << (unsigned int) image << ", ";
    }
    cout << "]" << endl;
}

void Dicom::setImage(uint16_t value)
{
    image_.push_back(value);
}

void DicomData::set_tag(uint8_t tag[4])
{
    char group[8], element[8];
    sprintf(group, "%02x%02x", tag[1], tag[0]);
    sprintf(element, "%02x%02x", tag[3], tag[2]);
    tag_.group = group;
    tag_.element = element;
}

void DicomData::set_vr(char tag[2])
{
    vr_ = tag[0];
    vr_ += tag[1];
}

void DicomData::set_length(int length)
{
    length_ = length;
}

