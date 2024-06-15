#include <iostream>
#include <fstream>
#include <string>
#include <cmath>
#include <iomanip>
#include <windows.h>
#include <vector>
#include <algorithm>
#include <sstream>

#pragma execution_character_set( "utf-8" )

using namespace std;

enum class Action
{
    Quit,
    MakeBD,
    CalculateWithPreparedData,
    FindMeasurements,
};

double Pi = atan(1) * 4;
const double eps = 1e-12;

//double U_m, U_z, U_p, U_0;

double U_z_min = 0.3, U_z_max = 0.9, U_z_step = 0.1;
double U_p_min = 0.1, U_p_max = 0.9, U_p_step = 0.1;
double U_0_min = -0.9, U_0_max = 0.9, U_0_step = 0.1;
double Time_min = 0, Time_max = 0.16;
double Delta_Deviation_Percent = 0.03;
double Number_Of_Matches_To_Write_To_file = 9;

double Pi_time = 0.01;
double Time_step = 0.00005;
int Pi_steps_count = round(Pi_time / Time_step);

void Make_DB(string File_name);
void Make_Data(ofstream& file, double _U_z, double _U_p, double _U_0, double _U_m = 1);
void Make_Data_Discharges_And_Deltas(ofstream& file1, ofstream& file2, double _U_z, double _U_p, double _U_0, double _U_m = 1);
void Find_Measurements(ifstream& file, vector<double> measurements);
bool Compare_Deltas(double db_delta, double measured_delta);
vector<double> Get_Deltas_From_Measurements(vector<double> measurements);

string Measurements_File_Name = "Measurements.txt";

int main()
{
    double U_m, U_z, U_p, U_0;
    int user_input;
    bool ProgramRunning = true;

    SetConsoleOutputCP(65001);

    while (ProgramRunning)
    {
        std::cout << "Чтобы создать базу данных ЧР, введите 1" << '\n' << '\n';
        std::cout << "Чтобы расчитать ЧР с заданными значениями, введите 2" << '\n' << '\n';
        std::cout << "Чтобы выполнить поиск измеренных ЧР в базе данных, введите 3" << '\n';
        std::cout << "Файл с измеренными значениями времени ЧР должен лежать в директории программы с названием " + Measurements_File_Name << '\n' << '\n';
        std::cout << "Чтобы выйти, введите 0" << '\n' << '\n';
        cin >> user_input;

        switch (static_cast<Action>(user_input))
        {
        case Action::Quit:
            ProgramRunning = false;
            break;
        case Action::MakeBD:
            Make_DB("DB");
            break;
        case Action::CalculateWithPreparedData:
        {
            std::cout << "Введите Uz, Up, Uo (через пробел, для разделения целой и дробной части используйте точку)" << '\n' << '\n';
            cin >> U_z >> U_p >> U_0;

            ofstream file;
            string filename = "Uz" + to_string(U_z) + "Up" + to_string(U_p) + "Uo" + to_string(U_0);
            file.open(filename + ".txt");
            Make_Data(file, U_z, U_p, U_0);
            file.close();
            std::cout << "В папке с программой создан файл с данными: " << filename + ".txt" << '\n' << '\n';

            ofstream file_discharge;
            ofstream file_discharge_delta_time;
            file_discharge.open(filename + "_discharge" + ".txt");
            file_discharge_delta_time.open(filename + "_discharge_delta_time" + ".txt");
            Make_Data_Discharges_And_Deltas(file_discharge, file_discharge_delta_time, U_z, U_p, U_0);
            file_discharge.close();
            file_discharge_delta_time.close();
            std::cout << "В папке с программой создан файл с данными о разрядах: " << filename + "_discharge" + ".txt" << '\n' << '\n';
            std::cout << "В папке с программой создан файл с данными о дельтах разрядов: " << filename + "_discharge_delta_time" + ".txt" << '\n' << '\n';
            break;
        }
        case Action::FindMeasurements:
        {
            ifstream measurements_file(Measurements_File_Name);

            vector<double> Measurements;
            double _measurement;

            while (measurements_file >> _measurement)
            {
                Measurements.push_back(_measurement);
            }
            sort(begin(Measurements), end(Measurements));

            measurements_file.close();

            ifstream Discharges_Delta_Time("DB_Discharges_Delta_Time.txt");

            Find_Measurements(Discharges_Delta_Time, Measurements);

            break;
        }
        default:
            std::cout << "Введено неверное значение" << '\n' << '\n';
        }
    }
    return 0;
}

void Make_Data(ofstream& file, double _U_z, double _U_p, double _U_0, double _U_m)
{
    double current_time, f_m, f_m_last, f, f_last;

    for (int i = 0; i <= Time_max / Time_step; i += 1)
    {
        current_time = i * Time_step;
        f_m = sin(current_time * Pi * 100);

        if (i == 0)
        {
            f = _U_0 + f_m;
        }
        else
        {
            if (Pi_steps_count / 2 < i % (2 * Pi_steps_count) && i % (2 * Pi_steps_count) <= Pi_steps_count / 2 * 3)
            {
                f_last + (f_m - f_m_last) <= -_U_z ? f = f_last + (_U_z - _U_p) + (f_m - f_m_last) : f = f_last + (f_m - f_m_last);
            }
            else
            {
                f_last + (f_m - f_m_last) >= _U_z ? f = f_last - (_U_z - _U_p) + (f_m - f_m_last) : f = f_last + (f_m - f_m_last);
            }
        }

        f_m_last = f_m;
        f_last = f;

        file << current_time << '\t' << f_m << '\t' << f << '\n';
    }
}

void Make_DB(string File_name)
{
    double U_m, U_z, U_p, U_0;

    ofstream file1;
    file1.open(File_name + ".txt");

    ofstream file2;
    file2.open(File_name + "_Discharges.txt");

    ofstream file3;
    file3.open(File_name + "_Discharges_Delta_Time.txt");


    for (double _U_z = U_z_min; _U_z <= U_z_max; _U_z += U_z_step)
    {
        for (double _U_p = U_p_min; _U_p <= U_p_max; _U_p += U_p_step)
        {
            for (double _U_0 = U_0_min; _U_0 <= U_0_max; _U_0 += U_0_step)
            {
                U_z = _U_z;
                U_p = _U_p * U_z;
                U_0 = _U_0 * U_z;

                std::cout << "Generating data for Uz = " << U_z << " , Up = " << U_p << " , U0 = " << U_0 << '\n';

                file1 << "Uz = " << U_z << " Up = " << U_p << " U0 = " << U_0 << '\n' << '\n';
                Make_Data(file1, U_z, U_p, U_0);
                file1 << '\n';

                file2 << "Uz = " << U_z << " Up = " << U_p << " U0 = " << U_0 << '\n' << '\n';
                file3 << U_z << " " << U_p << " " << U_0 << '\n';
                Make_Data_Discharges_And_Deltas(file2, file3, U_z, U_p, U_0);
                file2 << '\n';
            }
        }
    }
    file1.close();
}

void Compare_With_Last_Time(ofstream& file, double last_charge_time, double current_time)
{
    if (last_charge_time != 0)
    {
        file << current_time - last_charge_time << '\n';
    }
}

void Make_Data_Discharges_And_Deltas(ofstream& file1, ofstream& file2, double _U_z, double _U_p, double _U_0, double _U_m)
{
    double current_time, f_m, f_m_last, f, f_last, last_charge_time = 0;

    for (int i = 0; i <= Time_max / Time_step; i += 1)
    {
        current_time = i * Time_step;
        f_m = sin(current_time * Pi * 100);

        if (i == 0)
        {
            f = _U_0 + f_m;
        }
        else
        {
            if (Pi_steps_count / 2 < i % (2 * Pi_steps_count) && i % (2 * Pi_steps_count) <= Pi_steps_count / 2 * 3)
            {
                if (f_last + (f_m - f_m_last) <= -_U_z)
                {
                    file1 << current_time << '\n';
                    f = f_last + (_U_z - _U_p) + (f_m - f_m_last);
                    Compare_With_Last_Time(file2, last_charge_time, current_time);
                    last_charge_time = current_time;
                }
                else
                {
                    f = f_last + (f_m - f_m_last);
                }
            }
            else
            {
                if (f_last + (f_m - f_m_last) >= _U_z)
                {
                    file1 << current_time << '\n';
                    f = f_last - (_U_z - _U_p) + (f_m - f_m_last);
                    Compare_With_Last_Time(file2, last_charge_time, current_time);
                    last_charge_time = current_time;
                }
                else
                {
                    f = f_last + (f_m - f_m_last);
                }
            }
        }

        f_m_last = f_m;
        f_last = f;
    }
}

vector<double> Get_Deltas_From_Measurements(vector<double> measurements)
{
    vector<double> measurements_deltas;

    for (int i = 1; i < measurements.size(); i++)
    {
        measurements_deltas.push_back(measurements[i] - measurements[i - 1]);
    }
    return measurements_deltas;
}

bool Compare_Deltas(double db_delta, double measured_delta)
{
    return abs(db_delta - measured_delta) <= db_delta * Delta_Deviation_Percent;
}

void Find_Measurements(ifstream& file, vector<double> measurements)
{
    string line, output_file_name = "Matches_With_DB.txt";
    double current_DB_delta, delta_between_measured_discharges, U_z, U_p, U_0, U_z_cache, U_p_cache, U_0_cache;
    bool streak_started = false, streak_ended = false, is_delta_founded = false, is_need_change_BD_delta_to_next = false;
    int first_discharge_index = 0, current_number_of_matches = 0;

    ofstream output_file;
    output_file.open(output_file_name);

    while (getline(file, line))
    {
        istringstream iss(line);

        if (iss >> U_z >> U_p >> U_0)
        {
            std::cout << "Идет сравнение с Uz = " << U_z << " Up = " << U_p << " Uo = " << U_0 << '\n';

            if (current_number_of_matches >= Number_Of_Matches_To_Write_To_file)
            {
                output_file << "Совпадение с Uz " << U_z_cache << " Up " << U_p_cache << " Uo " << U_0_cache << '\n';
            }

            first_discharge_index = 0;
            current_number_of_matches = 0;
            streak_started = false;
            streak_ended = false;
            is_need_change_BD_delta_to_next = false;

            U_z_cache = U_z;
            U_p_cache = U_p;
            U_0_cache = U_0;
        }
        else
        {
            istringstream iss(line);
            iss >> current_DB_delta;
            if (streak_ended) continue;
            is_delta_founded = false;
            is_need_change_BD_delta_to_next = false;
            while (first_discharge_index < measurements.size() - 1)
            {
                for (int second_discharge_index = first_discharge_index + 1; second_discharge_index < measurements.size(); second_discharge_index++)
                {
                    delta_between_measured_discharges = measurements[second_discharge_index] - measurements[first_discharge_index];
                    if (delta_between_measured_discharges > current_DB_delta + eps) break;
                    if (!Compare_Deltas(current_DB_delta, delta_between_measured_discharges)) continue;
                    streak_started = true;
                    is_delta_founded = true;
                    first_discharge_index = second_discharge_index - 1;
                    current_number_of_matches++;
                    is_need_change_BD_delta_to_next = true;
                    break;
                }
                first_discharge_index++;
                if (is_need_change_BD_delta_to_next) break;
            }
            if (streak_started && !is_delta_founded) streak_ended = true;
        }
    }
    std::cout << "Данные записаны в файл " + output_file_name << '\n' << '\n';
    output_file.close();
}