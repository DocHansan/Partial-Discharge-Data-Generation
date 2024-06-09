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

//double U_m, U_z, U_p, U_0;

double U_z_min = 0.3, U_z_max = 0.9, U_z_step = 0.1;
double U_p_min = 0.1, U_p_max = 0.9, U_p_step = 0.1;
double U_0_min = -0.9, U_0_max = 0.9, U_0_step = 0.1;
double Time_min = 0, Time_max = 0.16;
double Delta_Deviation_Percent = 0.03;

double Pi_time = 0.01;
double Time_step = 0.00005;
int Pi_steps_count = round(Pi_time / Time_step);

void Make_DB(string File_name);
void Make_Data(ofstream& file, double _U_z, double _U_p, double _U_0, double _U_m = 1);
void Make_Data_Discharges_And_Deltas(ofstream& file1, ofstream& file2, double _U_z, double _U_p, double _U_0, double _U_m = 1);
void Find_Measurements(ifstream& file, vector<double> measurements, vector<double> measured_deltas);
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
        cout << "Чтобы создать базу данных ЧР, введите 1" << '\n' << '\n';
        cout << "Чтобы расчитать ЧР с заданными значениями, введите 2" << '\n' << '\n';
        cout << "Чтобы выполнить поиск измеренных ЧР в базе данных, введите 3" << '\n';
        cout << "Файл с измеренными значениями времени ЧР должен лежать в директории программы с названием " + Measurements_File_Name << '\n' << '\n';
        cout << "Чтобы выйти, введите 0" << '\n' << '\n';
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
            cout << "Введите Uz, Up, Uo (через пробел, для разделения целой и дробной части используйте точку)" << '\n' << '\n';
            cin >> U_z >> U_p >> U_0;

            ofstream file;
            string filename = "Uz" + to_string(U_z) + "Up" + to_string(U_p) + "Uo" + to_string(U_0);
            file.open(filename + ".txt");
            Make_Data(file, U_z, U_p, U_0);
            file.close();
            cout << "В папке с программой создан файл с данными: " << filename + ".txt" << '\n' << '\n';

            ofstream file_discharge;
            ofstream file_discharge_delta_time;
            file_discharge.open(filename + "_discharge" + ".txt");
            file_discharge.open(filename + "_discharge_delta_time" + ".txt");
            Make_Data_Discharges_And_Deltas(file_discharge, file_discharge_delta_time, U_z, U_p, U_0);
            file_discharge.close();
            cout << "В папке с программой создан файл с данными о разрядах: " << filename + "_discharge" + ".txt" << '\n' << '\n';
            break;
        }
        case Action::FindMeasurements:
        {
            ifstream measurements_file("Measurements.txt");

            vector<double> Measurements;
            double _measurement;

            while (measurements_file >> _measurement)
            {
                Measurements.push_back(_measurement);
            }

            measurements_file.close();

            ifstream Discharges_Delta_Time("DB_Discharges_Delta_Time.txt");

            Find_Measurements(Discharges_Delta_Time, Measurements, Get_Deltas_From_Measurements(Measurements));

            break;
        }
        default:
            cout << "Введено неверное значение" << '\n' << '\n';
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

                cout << "Generating data for Uz = " << U_z << " , Up = " << U_p << " , U0 = " << U_0 << '\n';

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

void Write_Delta_In_File(ofstream& file, double *first_discharge_time, double current_time)
{
    if (*first_discharge_time == 0)
    {
        *first_discharge_time = current_time;
        return;
    }
    file << current_time - (*first_discharge_time) << '\n';
}

void Make_Data_Discharges_And_Deltas(ofstream& file1, ofstream& file2, double _U_z, double _U_p, double _U_0, double _U_m)
{
    double current_time, f_m, f_m_last, f, f_last, first_discharge_time = 0;

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
                    Write_Delta_In_File(file2, &first_discharge_time, current_time);
                }
                else
                {
                    f = f_last + (f_m - f_m_last);
                }
            }
            else
            {
                /*if (!Is_Upward_Direction)
                {
                    Is_Upward_Direction = !Is_Upward_Direction;
                    last_charge_time = 0;
                }*/
                if (f_last + (f_m - f_m_last) >= _U_z)
                {
                    file1 << current_time << '\n';
                    f = f_last - (_U_z - _U_p) + (f_m - f_m_last);
                    Write_Delta_In_File(file2, &first_discharge_time, current_time);
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
    sort(begin(measurements), end(measurements));

    vector<double> measurements_deltas;

    for (int i = 1; i < measurements.size(); i++)
    {
        measurements_deltas.push_back(measurements[i] - measurements[0]);
    }
    return measurements_deltas;
}

void Find_Measurements(ifstream& file, vector<double> measurements, vector<double> measured_deltas)
{
    string line, output_file_name = "Matches_With_DB.txt";
    double current_delta, U_z, U_z_cache, U_p, U_0, measured_deltas_index;
    bool streak_of_matches = false, skip_to_next = false;

    ofstream output_file;
    output_file.open(output_file_name);

    /*for (auto i : measured_deltas)
    {
        cout << i << endl;
    }
    cout << endl << endl << endl;*/

    while (getline(file, line))
    {
        istringstream iss(line);

        if (iss >> U_z >> U_p >> U_0)
        {
            U_z_cache = U_z;
            cout << "Идет сравнение с Uz = " << U_z << " Up = " << U_p << " Uo = " << U_0 << '\n';
            measured_deltas_index = 0;
            streak_of_matches = false;
            skip_to_next = false;
        }
        else
        {
            istringstream iss(line);
            if (iss >> current_delta)
            {
                //cout << current_delta << endl;
                if (skip_to_next) continue;
                cout << current_delta << "   " << measured_deltas[measured_deltas_index] << endl;
                if (Compare_Deltas(current_delta, measured_deltas[measured_deltas_index]))
                {
                    //cout << "true" << endl;
                    if (streak_of_matches)
                    {
                        if (measured_deltas_index == measured_deltas.size() - 1)
                        {
                            cout << "Совпадение с Uz " << U_z_cache << " Up " << U_p << " Uo " << U_0 << '\n';
                            output_file << "Совпадение с Uz " << U_z_cache << " Up " << U_p << " Uo " << U_0 << '\n';
                            skip_to_next = true;
                        }

                    }
                    else
                    {
                        streak_of_matches = true;
                    }
                    measured_deltas_index++;
                }
                else
                {
                    //cout << "false" << endl;
                    measured_deltas_index = 0;
                    streak_of_matches = false;
                }
            }
        }
    }
    cout << "Данные записаны в файл " + output_file_name << '\n' << '\n';
    output_file.close();
}

bool Compare_Deltas(double db_delta, double measured_delta)
{
    return measured_delta <= db_delta * (1 + Delta_Deviation_Percent) && measured_delta >= db_delta * (1 - Delta_Deviation_Percent);
}