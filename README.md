The Timetable Generator is an automated scheduling system that generates an optimized timetable using a CSV file uploaded by the user.
Instead of manually creating timetables—which is time-consuming and error-prone—this system processes structured input data and produces a conflict-free timetable automatically.

The project is designed for educational institutes, supporting multiple teachers, subjects, classes, labs, and time slots.

#Key Features
* CSV File Upload – Users upload input data in CSV format
* Automatic Timetable Generation – No manual scheduling required
* Conflict-Free Scheduling – Prevents teacher, class, and lab clashes
* Multi-Teacher & Multi-Class Support
* Lab & Theory Period Handling
* Structured Output – Generates a well-organized timetable
* Reusability – Easily regenerate timetable by uploading a new CSV

Input Format (CSV File)
The system accepts a CSV file containing timetable constraints and academic data.
Field Description:
Column Name	                    |  Description
Teacher_Name	                  |  Name of the faculty
Subject	                        |  Subject name
Class	                          |  Year or standard
Division	                      |  Class division
Hours_Per_Week	                |  Required weekly lectures
Type	                          |  Theory / Lab

How It Works
1.User uploads a CSV file
2.System validates input data
3.Scheduling algorithm processes constraints
4.Time slots are allocated dynamically
5.Final timetable is generated in csv
