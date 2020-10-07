# ClauParser  [![Build status](https://ci.appveyor.com/api/projects/status/eq8cfklovioh7fj9?svg=true)](https://ci.appveyor.com/project/vztpv/clauparser)[![Total alerts](https://img.shields.io/lgtm/alerts/g/ClauParser/ClauParser.svg?logo=lgtm&logoWidth=18)](https://lgtm.com/projects/g/ClauParser/ClauParser/alerts/)[![Language grade: C/C++](https://img.shields.io/lgtm/grade/cpp/g/ClauParser/ClauParser.svg?logo=lgtm&logoWidth=18)](https://lgtm.com/projects/g/ClauParser/ClauParser/context:cpp)

My Own "Parallel Parser" for "Paradox Game Data" (clausewitz engine or jomini engine)

# 
Load Data Parallel From Paradox Game Data File ( especially "save game" )and Create, Read, Update, Delete Data, and Save Data to File.

 

Loading Data Parallel From Paradox Game Data File(50MB) take 360 ~ 600 ms in my computer.(Intel i7 8th gen cpu - 4 core 8 thread,, ssd)

#
Needs C++17. ( but files in cpp11 folder need C++11 )

Supports UTF-8(BOM).

Supports Linux(g++), Windows(Visual C++).


#

Following is supported Data File Example.
#
    #line comment

    EU4Game

    Countries = {

        -1 = {

            test = 1 test2 = { 3 4 5 6 7 }

       }

    }



And Principle of Loading Data From File Parallel (Simply)

Lexing(Scanning) : // split data by '\n', Scanning Parallel, Merge Tokens.

Parsing : split data by (including) '{', '}', val in var = val, val in val, Parsing(make tree) Parallel and Merge Trees.



(in 'x = 1', 'x' is var, '1' is val)

(in '2 3 4 5', '2' is val, '3' is val, '4' is val, '5' is val.

I regarded var of '2' as empty string.)



(so, 'x = 1' and '2' are presented using same C++ Class,

if var is empty string, then no print var and '='), I named this type as 'ItemType'')



(and '-1 = { }', ' { } ' is form of var = { }, '{ }' `s var is regarded as empty string.)



( I named this type as 'UserType' )

(and 'UserType' also used to include total data.)



(Code Example Using This Parser

UserType global;

// here UserType is used for Total Data,

// UserType has std::vector of 'ItemType'

// and has std::vector of 'UserType*'

LoadData::LoadDataFromFile("input.eu4", global, 0, 0); // 0 means use all cpu cores. )



Parallel Parsing - It use 'Virtual Node', and Line comments start from '#',

I used Virtual Node`s Name as '#'



For example, x = 1 x = { 2 3 4 5 }

=> x = 1 x = { 2 // if here split.

3 4 5 }

=> (1st tree) x = 1 x = { 2 }

(2nd tree) # = { 3 4 5 }



Merge in implementation, program stored tree`s split position.



so, using split position of 1st tree and '#' (virtual node) of 2nd tree

=> it can be to merge 1st tree and 2nd tree.



In Implementation, This Parser uses n thread in Loading Data From File.

and In this Implementation, Line Comments are disappeared, when Load Data From File. 
