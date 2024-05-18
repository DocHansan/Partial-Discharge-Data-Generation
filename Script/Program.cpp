#include <iostream>
#include <fstream>
#include <string>
#include <cmath>
#include <iomanip>
#include <windows.h>

#pragma execution_character_set( "utf-8" )

using namespace std;

enum class Action
{
    Quit,
    MakeBD,
    CalculateWithPreparedData,
};

double Pi = atan(1) * 4;

//double U_m, U_z, U_p, U_0;

double U_z_min = 0.3, U_z_max = 0.9, U_z_step = 0.1;
double U_p_min = 0.1, U_p_max = 0.9, U_p_step = 0.1;
double U_0_min = -0.9, U_0_max = 0.9, U_0_step = 0.1;
double Time_min = 0, Time_max = 0.16;

double Pi_time = 0.01;
double Time_step = 0.00005;
int Pi_steps_count = round(Pi_time / Time_step);

void Make_DB(string File_name);
void Make_Data(ofstream& file, double _U_z, double _U_p, double _U_0, double _U_m = 1);
void Make_Data_Find_Discharge(ofstream& file1, ofstream& file2, double _U_z, double _U_p, double _U_0, double _U_m = 1);

int main()
{
    double U_m, U_z, U_p, U_0;
    int user_input;
    bool ProgramRunning = true;

    SetConsoleOutputCP(65001);

    while (ProgramRunning)
    {
        cout << "Чтобы создать базу данных ЧР, введите 1" << '\n';
        cout << "Чтобы расчитать ЧР с заданными значениями, введите 2" << '\n';
        cout << "Чтобы выйти, введите 0" << '\n';
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
            cout << "Введите Uz, Up, Uo (через пробел, для разделения целой и дробной части используйте точку)" << '\n';
            cin >> U_z >> U_p >> U_0;

            ofstream file;
            string filename = "Uz" + to_string(U_z) + "Up" + to_string(U_p) + "Uo" + to_string(U_0);
            file.open(filename + ".txt");
            Make_Data(file, U_z, U_p, U_0);
            file.close();
            cout << "В папке с программой создан файл с данными: " << filename + ".txt" << '\n';

            ofstream file_discharge;
            ofstream file_discharge_delta_time;
            file_discharge.open(filename + "_discharge" + ".txt");
            file_discharge.open(filename + "_discharge_delta_time" + ".txt");
            Make_Data_Find_Discharge(file_discharge, file_discharge_delta_time, U_z, U_p, U_0);
            file_discharge.close();
            cout << "В папке с программой создан файл с данными о разрядах: " << filename + "_discharge" + ".txt" << '\n';
            break;
        }
        default:
            cout << "Введено неверное значение" << '\n';
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
                file3 << "Uz = " << U_z << " Up = " << U_p << " U0 = " << U_0 << '\n' << '\n';
                Make_Data_Find_Discharge(file2, file3, U_z, U_p, U_0);
                file2 << '\n';
                file3 << '\n';
            }
        }
    }
    file1.close();
}

double Compare_With_Last_Time(ofstream& file, double last_charge_time, double current_time)
{
    if (last_charge_time == 0)
    {
        last_charge_time = current_time;
    }
    else
    {
        file << current_time - last_charge_time << '\n';
        last_charge_time = current_time;
    }
    return last_charge_time;
}

void Make_Data_Find_Discharge(ofstream& file1, ofstream& file2, double _U_z, double _U_p, double _U_0, double _U_m)
{
    double current_time, f_m, f_m_last, f, f_last, last_charge_time = 0;

    bool Is_Upward_Direction = true;

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
                if (Is_Upward_Direction)
                {
                    Is_Upward_Direction = !Is_Upward_Direction;
                    last_charge_time = 0;

                }
                if (f_last + (f_m - f_m_last) <= -_U_z)
                {
                    file1 << current_time << '\n';
                    f = f_last + (_U_z - _U_p) + (f_m - f_m_last);
                    last_charge_time = Compare_With_Last_Time(file2, last_charge_time, current_time);
                }
                else
                {
                    f = f_last + (f_m - f_m_last);
                }
            }
            else
            {
                if (!Is_Upward_Direction)
                {
                    Is_Upward_Direction = !Is_Upward_Direction;
                    last_charge_time = 0;
                }
                if (f_last + (f_m - f_m_last) >= _U_z)
                {
                    file1 << current_time << '\n';
                    f = f_last - (_U_z - _U_p) + (f_m - f_m_last);
                    last_charge_time = Compare_With_Last_Time(file2, last_charge_time, current_time);
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