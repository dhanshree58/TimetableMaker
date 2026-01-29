#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <sstream>
#include <map>
#include <algorithm>
#include <iomanip>

using namespace std;

// Enumerations for better code readability and structure
enum SlotType { LECTURE, LAB, TUTORIAL };
enum Day { MON, TUE, WED, THU, FRI, SAT, SUN };

// Forward declaration of the increment operator for Day
Day operator++(Day& d);

// Helper function to convert Day enum to string
string day_to_string(Day d);

// --- 1. Subject Class ---
class Subject {
public:
    string name;
    SlotType type; // LECTURE or LAB
    int theory_lectures; // e.g., 2
    int lab_slots;       // e.g., 1 (for 2-hour slot)

    Subject(string n, SlotType t, int lec, int lab)
        : name(n), type(t), theory_lectures(lec), lab_slots(lab) {}
};

// --- 2. TimetableEntry Class ---
class TimetableEntry {
public:
    Day day;
    int hour_slot; // e.g., 9 (for 9:00-10:00)
    string division; // e.g., SY-1
    char batch;      // e.g., 'A' (or 'X' for common lecture)
    string subject_name;
    string teacher_name;
    SlotType type;

    string toString() const;
};

// --- 3. Teacher Class ---
class Teacher {
public:
    string name;
    int total_load;
    int remaining_load;
    map<int, map<Day, string>> schedule; // hour_slot -> Day -> SubjectName
    vector<string> theory_subjects;
    vector<string> lab_subjects;
    string work_days_str; // e.g., "Mon-Fri"

    Teacher(string n, int load, const vector<string>& theory, const vector<string>& lab, string days)
        : name(n), total_load(load), remaining_load(load), theory_subjects(theory), lab_subjects(lab), work_days_str(days) {
        // Initialize the schedule map (up to 18:00 or 10-6 = 8 slots)
        for (int h = 8; h <= 18; ++h) {
            for (int d = MON; d <= FRI; ++d) {
                schedule[h][(Day)d] = ""; // "" means available
            }
        }
    }

    bool is_available(Day d, int hour_slot, int duration = 1) const;
    void assign_slot(Day d, int hour_slot, int duration, const string& subject_name);
    bool teaches_theory(const string& subject) const;
    bool teaches_lab(const string& subject) const;
};

// --- 4. TimetableGenerator Class (The Engine) ---
class TimetableGenerator {
private:
    vector<Teacher> teachers;
    vector<Subject> subjects;
    vector<TimetableEntry> timetable;
    vector<string> division_names;
    map<string, map<char, map<string, int>>> remaining_slots; // Division -> Batch -> SubjectName -> Count

    void initialize_slots();
    bool try_assign_lecture(Day d, int hour, const string& division);
    bool try_assign_lab(Day d, int hour, const string& division);
    Teacher* find_available_teacher(Day d, int hour, const string& subject, SlotType type, int duration = 1);
    bool try_assign_diff_labs_to_batches(Day d, int hour, const string& division);



public:
    TimetableGenerator();
    bool load_data_from_file(const string& filename);
    void generate_timetable();
    void save_timetable_to_file(const string& filename);
    void print_timetable_by_division(const string& div_name) const;
};