#include "Timetable.h"

int main() {
    // 1. Create the generator object
    TimetableGenerator generator;
    const string input_file = "teachers_data.csv";
    const string output_file = "sy_timetable_output.csv";

    // 2. Load teacher data (File Handling: Input)
    if (!generator.load_data_from_file(input_file)) {
        return 1; // Exit if file reading fails
    }

    // 3. Generate the timetable (OOP Logic)
    generator.generate_timetable();

    // 4. Save the results (File Handling: Output)
    generator.save_timetable_to_file(output_file);
    
    // 5. Verification 
    generator.print_timetable_by_division("SY-1");
     generator.print_timetable_by_division("SY-2");
      generator.print_timetable_by_division("SY-3");
       generator.print_timetable_by_division("SY-4");
        generator.print_timetable_by_division("SY-5");
         generator.print_timetable_by_division("SY-6");
          generator.print_timetable_by_division("SY-7");
           generator.print_timetable_by_division("SY-8");
            generator.print_timetable_by_division("SY-9");
             generator.print_timetable_by_division("SY-10");
    generator.print_timetable_by_division("SY-11");
        generator.print_timetable_by_division("SY-12");

    return 0;
}