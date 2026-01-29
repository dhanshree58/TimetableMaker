#include "Timetable.h"

// FIX 1: Implement custom increment operator for the Day enum (required for for loop)
Day operator++(Day& d) {
    // Cast the current enum value to int, increment, and cast back.
    d = static_cast<Day>(static_cast<int>(d) + 1);
    return d;
}

// Implementation of helper function
string day_to_string(Day d) {
    static const vector<string> names = {"MON", "TUE", "WED", "THU", "FRI", "SAT", "SUN"};
    if (d >= MON && d <= SUN) {
        return names[d];
    }
    return "INVALID";
}

// Implementation of TimetableEntry method
string TimetableEntry::toString() const {
    string type_str = (type == LECTURE) ? "Lec" : (type == LAB ? "Lab" : "Tut");
    string batch_str = (batch == 'X') ? "All" : string(1, batch);
    return day_to_string(day) + "," + to_string(hour_slot) + "-" + to_string(hour_slot + (type == LAB ? 2 : 1)) +
           "," + division + "-" + batch_str + "," + subject_name + "," + teacher_name + "," + type_str;
}

// Implementation of Teacher methods
bool Teacher::is_available(Day d, int hour_slot, int duration) const {
    // 1. Check working days constraint
    if ((work_days_str == "Mon-Fri" && (d == SAT || d == SUN)) ||
        (work_days_str == "Tue-Sat" && d == MON)) {
        return false;
    }
    
    // 2. Check total remaining load
    if (remaining_load < duration) return false;

    // 3. Check time slot availability (no double booking)
    for (int i = 0; i < duration; ++i) {
        // Check within the working days (MON-FRI) schedule only
        if (d >= MON && d <= FRI && schedule.at(hour_slot + i).at(d) != "") {
            return false;
        }
    }
    return true;
}

void Teacher::assign_slot(Day d, int hour_slot, int duration, const string& subject_name) {
    for (int i = 0; i < duration; ++i) {
        schedule[hour_slot + i][d] = subject_name;
    }
    remaining_load -= duration;
}

bool Teacher::teaches_theory(const string& subject) const {
    return find(theory_subjects.begin(), theory_subjects.end(), subject) != theory_subjects.end();
}

bool Teacher::teaches_lab(const string& subject) const {
    return find(lab_subjects.begin(), lab_subjects.end(), subject) != lab_subjects.end();
}

// Implementation of TimetableGenerator methods
TimetableGenerator::TimetableGenerator() {
    // Define the 4 SY subjects (DS, OOPS, LDAM, DBMS)
    subjects.emplace_back("DS", LECTURE, 2, 0);   // 2 Lectures
    subjects.emplace_back("OOPS", LECTURE, 2, 0); // 2 Lectures
    subjects.emplace_back("LDAM", LECTURE, 2, 0); // 2 Lectures
    subjects.emplace_back("DBMS", LECTURE, 2, 0); // 2 Lectures

    // Corresponding Lab Subjects (1 slot / 2 hours)
    subjects.emplace_back("DSLAB", LAB, 0, 1); 
    subjects.emplace_back("OOPSLAB", LAB, 0, 1);
    subjects.emplace_back("LDAMLAB", LAB, 0, 1);
    subjects.emplace_back("DBMSLAB", LAB, 0, 1);
    
    // Define 12 divisions
    for (int i = 1; i <= 12; ++i) {
        division_names.push_back("SY-" + to_string(i));
    }
    
    initialize_slots();
}

// Initialize the remaining slots map 
void TimetableGenerator::initialize_slots() {
    for (const auto& div : division_names) {
        for (char batch : {'A', 'B', 'C'}) {
            for (const auto& sub : subjects) {
                if (sub.type == LECTURE) {
                    remaining_slots[div][batch][sub.name] = sub.theory_lectures; 
                } else if (sub.type == LAB) {
                    remaining_slots[div][batch][sub.name] = sub.lab_slots;       
                }
            }
        }
    }
}

// Reading Teacher Data 
bool TimetableGenerator::load_data_from_file(const string& filename) {
    ifstream file(filename);
    if (!file.is_open()) {
        cerr << "Error: Could not open file " << filename << endl;
        return false;
    }

    string line;
    getline(file, line);

    while (getline(file, line)) {
        stringstream ss(line);
        string token;
        vector<string> parts;
        while (getline(ss, token, ',')) {
            parts.push_back(token);
        }

        if (parts.size() < 8) continue; // Basic validation

        string name = parts[0];
        // FIX 2: Using std::stoi is safer, requires C++11 flag or proper scope
        int total_load = std::stoi(parts[1]); 

        // Helper to split subjects string (e.g., "DS;OOPS" -> {"DS", "OOPS"})
        auto split_subjects = [](const string& s) {
            vector<string> subs;
            stringstream sub_ss(s);
            string sub_token;
            while (getline(sub_ss, sub_token, ';')) {
                subs.push_back(sub_token);
            }
            return subs;
        };

        vector<string> theory_subs = split_subjects(parts[5]);
        vector<string> lab_subs = split_subjects(parts[6]);
        string work_days = parts[7];

        teachers.emplace_back(name, total_load, theory_subs, lab_subs, work_days);
    }
    cout << "Loaded " << teachers.size() << " teachers successfully." << endl;
    return true;
}

// --- CORE GENERATION LOGIC ---

Teacher* TimetableGenerator::find_available_teacher(Day d, int hour, const string& subject, SlotType type, int duration) {
    for (auto& teacher : teachers) {
        bool teaches = (type == LECTURE) ? teacher.teaches_theory(subject) : teacher.teaches_lab(subject);
        
        if (teaches && teacher.is_available(d, hour, duration)) {
            return &teacher;
        }
    }
    return nullptr;
}

// Attempts to assign a 1-hour lecture to the entire division
bool TimetableGenerator::try_assign_lecture(Day d, int hour, const string& division) {
    for (const auto& sub_pair : remaining_slots[division]['A']) {
        const string& subject_name = sub_pair.first;
        int remaining = sub_pair.second;

        if (remaining > 0 && subject_name.find("LAB") == string::npos) { 
            Teacher* teacher = find_available_teacher(d, hour, subject_name, LECTURE, 1);

            if (teacher) {
                teacher->assign_slot(d, hour, 1, subject_name);
                timetable.push_back({d, hour, division, 'X', subject_name, teacher->name, LECTURE});

                for(char batch : {'A', 'B', 'C'}) {
                    remaining_slots[division][batch][subject_name]--;
                }
                return true;
            }
        }
    }
    return false;
}

// Attempts to assign a 2-hour lab slot to the three batches (A, B, C) simultaneously
bool TimetableGenerator::try_assign_lab(Day d, int hour, const string& division) {
    const int LAB_DURATION = 2;
    
    if (hour > 16) return false; // Must start by 16:00 to finish by 18:00

    vector<string> lab_subs = {"DSLAB", "OOPSLAB", "LDAMLAB", "DBMSLAB"};

    for (const string& subject_name : lab_subs) {
        if (remaining_slots[division]['A'][subject_name] > 0) {
            
            vector<Teacher*> assigned_teachers;
            
            // FIX 3: Initialize available_teachers as a list of pointers
            vector<Teacher*> available_teachers; 
            for (auto& teacher : teachers) {
                available_teachers.push_back(&teacher);
            }

            // Look for 3 teachers
            for (int i = 0; i < 3; ++i) { 
                Teacher* t = nullptr;
                for(auto teacher_ptr : available_teachers) {
                    
                    // FIX 4: Use -> (arrow operator) for pointer access
                    if (teacher_ptr->teaches_lab(subject_name) && 
                        teacher_ptr->is_available(d, hour, LAB_DURATION) && 
                        find(assigned_teachers.begin(), assigned_teachers.end(), teacher_ptr) == assigned_teachers.end()) {
                        
                        t = teacher_ptr; // FIX 5: teacher_ptr is already a Teacher*
                        break;
                    }
                }
                if (t) {
                    assigned_teachers.push_back(t);
                }
            }

            // If we found 3 teachers, assign the slots
            if (assigned_teachers.size() == 3) {
                char batch = 'A';
                for (Teacher* teacher : assigned_teachers) {
                    teacher->assign_slot(d, hour, LAB_DURATION, subject_name);
                    timetable.push_back({d, hour, division, batch, subject_name, teacher->name, LAB});
                    remaining_slots[division][batch][subject_name]--;
                    batch++;
                }
                return true;
            }
        }
    }
    return false;
}


bool TimetableGenerator::try_assign_diff_labs_to_batches(Day d, int hour, const string& division) {
      vector<char> batches = {'A', 'B', 'C'}; // Each batch gets different lab
    for (char batch : batches) {
        for (auto& subj : subjects) {
            if (subj.type == LAB && remaining_slots[division][batch][subj.name] > 0) {
                Teacher* t = find_available_teacher(d, hour, subj.name, LAB, subj.lab_slots);
                if (t) {
                    t->assign_slot(d, hour, subj.lab_slots, subj.name);
                    remaining_slots[division][batch][subj.name]--;
                    timetable.push_back({d, hour, division, batch, subj.name, t->name, LAB});
                    break; // one lab per batch per slot
                }
            }
        }
    }
}

void TimetableGenerator::generate_timetable() {

    cout << "\nStarting Timetable Generation (1 lab + 2 lectures per day per division)..." << endl;

    const int START_HOUR = 8;
    const int END_HOUR = 17; // Last lecture can start at 17 for 1-hour

    for (Day d = MON; d <= THU; ++d) { // 4 working days
        for (const auto& div_name : division_names) {

            int labs_assigned_today = 0;      // 1 lab per day
            int lectures_assigned_today = 0;  // 2 lectures per day

            for (int hour = START_HOUR; hour <= END_HOUR; ++hour) {

                // --- PRIORITY 1: Assign 1 lab session (2-hour block) ---
                if (labs_assigned_today < 1 && hour <= END_HOUR - 1) { // check room for 2-hour lab
                    if (try_assign_diff_labs_to_batches(d, hour, div_name)) {
                        labs_assigned_today++;
                        hour += 1; // skip next hour for 2-hour lab
                        continue;
                    }
                }

                // --- PRIORITY 2: Assign theory lectures (1-hour) ---
                if (lectures_assigned_today < 2) {
                    if (try_assign_lecture(d, hour, div_name)) {
                        lectures_assigned_today++;
                        continue;
                    }
                }

                // Stop early if daily quotas are met
                if (labs_assigned_today >= 1 && lectures_assigned_today >= 2) {
                    break;
                }
            }

            // Fallback: try remaining hours if some slots couldn't be assigned
            if (labs_assigned_today < 1 || lectures_assigned_today < 2) {
                for (int hour = START_HOUR; hour <= END_HOUR; ++hour) {

                    if (labs_assigned_today < 1 && hour <= END_HOUR - 1) {
                        if (try_assign_diff_labs_to_batches(d, hour, div_name)) {
                            labs_assigned_today++;
                            hour += 1;
                        }
                    }

                    if (lectures_assigned_today < 2) {
                        if (try_assign_lecture(d, hour, div_name)) {
                            lectures_assigned_today++;
                        }
                    }

                    if (labs_assigned_today >= 1 && lectures_assigned_today >= 2) break;
                }
            }
        }
    }

    cout << "Generation complete. Total entries: " << timetable.size() << endl;
}


// --- FILE HANDLING: Saving Output ---
void TimetableGenerator::save_timetable_to_file(const string& filename) {
    ofstream file(filename);
    if (!file.is_open()) {
        cerr << "Error: Could not open output file " << filename << endl;
        return;
    }

    // Write header
    file << "Day,TimeSlot,Division-Batch,Subject,Teacher,Type\n";
    
    for (const auto& entry : timetable) {
        file << entry.toString() << "\n";
    }

    cout << "Timetable saved to " << filename << endl;
}

// Helper to print a specific division's timetable for verification
void TimetableGenerator::print_timetable_by_division(const string& div_name) const {
    cout << "\n--- Timetable for " << div_name << " ---\n";
    cout << left << setw(5) << "DAY" << setw(10) << "TIME" << setw(15) << "BATCH" << setw(10) << "SUB" << setw(20) << "TEACHER" << setw(5) << "TYPE" << endl;
    cout << string(65, '-') << endl;

    for (const auto& entry : timetable) {
        if (entry.division == div_name) {
            string batch_str = (entry.batch == 'X') ? "All" : string(1, entry.batch);
            string type_str = (entry.type == LECTURE) ? "Lec" : "Lab";
            cout << left << setw(5) << day_to_string(entry.day)
                 << setw(10) << to_string(entry.hour_slot) + "-" + to_string(entry.hour_slot + (entry.type == LAB ? 2 : 1))
                 << setw(15) << div_name + "-" + batch_str
                 << setw(10) << entry.subject_name
                 << setw(20) << entry.teacher_name
                 << setw(5) << type_str << endl;
        }
    }
}