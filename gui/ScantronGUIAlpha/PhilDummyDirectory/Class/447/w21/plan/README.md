# Scantron Creation GUI
> Generate exams in LaTex from a local database of questions.

One to two paragraph statement about your product and what it does.
The scantron creation GUI can be used to generate complete LaTex exam files using questions made by the user. 
The GUI allows for the unique naming of exams, along with the choice of how many unique exams to produce. Along with each
set of exams is an accompanying exam key which is used to grade completed student exams. 

## Compile

Modify the following fields in `createTest.c` for the current class:
```c
char *classnum;
char *qtr;
```

NetBSD & Linux:

Navigate to 

```sh
make
```

## Usage example


```sh
./scantroncreation [-c]
```

## Question Format


Questions shoulds be placed in a folder named after their topic within /test-db/. There are currently two types of questions supported: multiple choice and long answer. 

Multiple choice questions should begin with a 'Q-' and be followed by the question. The lines that follow should follow the form of 'A-"Possible Answer"' or 'X-"Correct Answer"'. 

```sh
Q-Question.
A-answer 1
A-answer 2
A-answer 3
A-answer 4
X-answer 5
```

Long answer questions should begin with a 'L-' and be followed by the question. Similarly to multiple choice questions, they can be followed by a verbatim.

```sh
L-Long answer style question.
V-Verbatim: Enter formulas and other stylized text here if needed
```

## Release History


* 0.1.0
    * The first proper release
* 0.0.1
    * Work in progress

## Meta

Cameron Wallace – wallac21@wwu.edu
Jake Friberg    - friberj3@wwu.edu 

Distributed under the MIT license. See ``LICENSE`` for more information.
