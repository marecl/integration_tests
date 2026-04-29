#ifndef NORMAL_LSEEK_H
#define NORMAL_LSEEK_H

#pragma once

#include "../fs_test.h"

#include <vector>

std::vector<OrbisInternals::offset_spec_t> normal_lseek_variants = {
    {.offset = 0, .whence = 0},                       //
    {.offset = -123, .whence = 0},                    //
    {.offset = 123456, .whence = 0},                  //
    {.offset = 60, .whence = 0},                      //
    {.offset = 0, .whence = 1},                       //
    {.offset = 24, .whence = 1},                      //
    {.offset = -24, .whence = 1},                     //
    {.offset = -6666, .whence = 1},                   //
    {.offset = 123456, .whence = 1},                  //
    {.offset = 0, .whence = 2},                       //
    {.offset = 123456, .whence = 2},                  //
    {.offset = 100, .whence = 2},                     //
    {.offset = -100, .whence = 2},                    //
    {.offset = -100000, .whence = 2},                 //
    {.offset = -normal_read_target - 1, .whence = 2}, //
    {.offset = -normal_read_target, .whence = 2},     //
    {.offset = -normal_read_target + 1, .whence = 2}, //
    // cursed ones
    {.offset = 0, .whence = 3},                       //
    {.offset = 8, .whence = 3},                       //
    {.offset = 16, .whence = 3},                      //
    {.offset = 24, .whence = 3},                      //
    {.offset = 32, .whence = 3},                      //
    {.offset = 40, .whence = 3},                      //
    {.offset = 10240, .whence = 3},                   //
    {.offset = -1024, .whence = 3},                   //
    {.offset = -10240, .whence = 3},                  //
    {.offset = -pfs_read_target - 1, .whence = 3},    //
    {.offset = -pfs_read_target, .whence = 3},        //
    {.offset = -pfs_read_target + 1, .whence = 3},    //
    {.offset = -normal_read_target - 1, .whence = 3}, //
    {.offset = -normal_read_target, .whence = 3},     //
    {.offset = -normal_read_target + 1, .whence = 3}, //
    {.offset = 0, .whence = 4},                       //
    {.offset = 8, .whence = 4},                       //
    {.offset = 16, .whence = 4},                      //
    {.offset = 24, .whence = 4},                      //
    {.offset = 32, .whence = 4},                      //
    {.offset = 40, .whence = 4},                      //
    {.offset = 10240, .whence = 4},                   //
    {.offset = -1024, .whence = 4},                   //
    {.offset = -10240, .whence = 4},                  //
    {.offset = -pfs_read_target - 1, .whence = 4},    //
    {.offset = -pfs_read_target, .whence = 4},        //
    {.offset = -pfs_read_target + 1, .whence = 4},    //
    {.offset = -normal_read_target - 1, .whence = 4}, //
    {.offset = -normal_read_target, .whence = 4},     //
    {.offset = -normal_read_target + 1, .whence = 4}, //
    //
    {.offset = s64(0xFFFFFFFFFFFFFFFF), .whence = 0}, //
    {.offset = s64(0xFFFFFFFFFFFFFFFE), .whence = 0}, //
    {.offset = 0xFFFFFFFFFFFFFF, .whence = 0},        //
    {.offset = 0xFFFFFFFFFFFFFE, .whence = 0},        //
    {.offset = 0xFFFFFFFFFFFF, .whence = 0},          //
    {.offset = 0xFFFFFFFFFFFE, .whence = 0},          //
    {.offset = 0xFFFFFFFFFF, .whence = 0},            //
    {.offset = 0xFFFFFFFFFE, .whence = 0},            //
    {.offset = 0xFFFFFFFF, .whence = 0},              //
    {.offset = 0xFFFFFFFE, .whence = 0},              //
    {.offset = 0xEFFFFFFF, .whence = 0},              //
    {.offset = 0xEFFFFFFE, .whence = 0},              //
    {.offset = 0xDFFFFFFF, .whence = 0},              //
    {.offset = 0xDFFFFFFE, .whence = 0},              //
    {.offset = 0xCFFFFFFF, .whence = 0},              //
    {.offset = 0xCFFFFFFE, .whence = 0},              //
    {.offset = 0xBFFFFFFF, .whence = 0},              //
    {.offset = 0xBFFFFFFE, .whence = 0},              //
    {.offset = 0xAFFFFFFF, .whence = 0},              //
    {.offset = 0xAFFFFFFE, .whence = 0},              //
    {.offset = 0xA0000000, .whence = 0},              //
    {.offset = 0x9FFFFFFF, .whence = 0},              //
    {.offset = 0x9FFFFFFE, .whence = 0},              //
    {.offset = 0x90000000, .whence = 0},              //
    {.offset = 0x8FFFFFFF, .whence = 0},              //
    {.offset = 0x8FFFFFFE, .whence = 0},              //
    {.offset = 0x80000000, .whence = 0},              //
    {.offset = 0x7FFFFFFF, .whence = 0},              //
    {.offset = 0x7FFFFFFE, .whence = 0},              //
    {.offset = 0x70000000, .whence = 0},              //
    {.offset = 0x6FFFFFFF, .whence = 0},              //
    {.offset = 0x6FFFFFFE, .whence = 0},              //
    {.offset = 0xFFFFFF, .whence = 0},                //
    {.offset = 0xFFFFFE, .whence = 0},                //
    {.offset = 0xFFFF, .whence = 0},                  //
    {.offset = 0xFFFE, .whence = 0},                  //
    {.offset = 0xFF, .whence = 0},                    //
    {.offset = 0xFE, .whence = 0},                    //
};

#endif // NORMAL_LSEEK_H