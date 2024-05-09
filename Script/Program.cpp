#include <iostream>
#include <fstream>
#include <string>
#include <cmath>
#include <iomanip>

using namespace std;

double Pi = atan(1) * 4;

double U_m, U_z, U_p, U_0, f_m, f_m_last, f, f_last, _Current_time;

double U_z_min = 0.3, U_z_max = 0.9;
double U_p_min = 0.1, U_p_max = 0.9;
double U_0_min = -0.9, U_0_max = 0.9;
double Time_min = 0, Time_max = 0.16;

double Pi_time = 0.01;
double Time_step = 0.00005;
int Pi_steps_count = Pi_time / Time_step;

int main()
{
    ofstream fout;
    fout.open("DB.txt");

    for (double _U_z = U_z_min; _U_z <= U_z_max; _U_z += 0.1)
    {
        for (double _U_p = U_p_min; _U_p <= U_p_max; _U_p += 0.1)
        {
            for (double _U_0 = U_0_min; _U_0 <= U_0_max; _U_0 += 0.1)
            {
                U_z = _U_z;
                U_p = _U_p * U_z;
                U_0 = _U_0 * U_z;

                fout << "Uz = " << U_z << " Up = " << U_p << " U0 = " << U_0 << '\n' << '\n';
                cout << "Generating data for Uz = " << U_z << " , Up = " << U_p << " , U0 = " << U_0 << '\n';

                for (int i = 0; i <= Time_max / Time_step; i += 1)
                {
                    _Current_time = i * Time_step;
                    f_m = sin(_Current_time * Pi * 100);

                    if (i == 0)
                    {
                        f = U_0 + f_m;
                    }
                    else
                    {
                        if (Pi_steps_count / 2 < i % (2 * Pi_steps_count) && i % (2 * Pi_steps_count) <= Pi_steps_count / 2 * 3)
                        {
                            f_last + (f_m - f_m_last) <= -U_z ? f = f_last + (U_z - U_p) + (f_m - f_m_last) : f = f_last + (f_m - f_m_last);
                        }
                        else
                        {
                            f_last + (f_m - f_m_last) >= U_z ? f = f_last - (U_z - U_p) + (f_m - f_m_last) : f = f_last + (f_m - f_m_last);
                        }
                    }

                    f_m_last = f_m;
                    f_last = f;

                    fout << _Current_time << '\t' << f_m << '\t' << f << '\n';
                }

                fout << '\n';

            }
        }
    }

    fout.close();

    return 0;
}