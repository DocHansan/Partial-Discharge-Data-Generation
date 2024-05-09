#include <iostream>
#include <fstream>
#include <string>
#include <cmath>

using namespace std;

double Pi = atan(1) * 4;

double U_m, U_z, U_p, U_0, f_m, f_m_last, f, f_last;

double Time_step = 0.005;

int main()
{
    ofstream fout;
    fout.open("DB.txt");

    for (double _U_z = 0.3; _U_z <= 0.9; _U_z += 0.1)
    {
        for (double _U_p = 0.1; _U_p <= 0.9; _U_p += 0.1)
        {
            for (double _U_0 = -0.9; _U_0 <= 0.9; _U_0 += 0.1)
            {
                U_z = _U_z;
                U_p = U_z * _U_p;
                U_0 = U_z * _U_0;

                fout << "Uz = " << U_z << ", Up = " << U_p << ", U0 = " << U_0 << '\n' << '\n';

                for (double i = 0; i <= 16; i += Time_step)
                {
                    f_m = sin(i * Pi);

                    if (i == 0)
                        f = U_0 + f_m;

                    else
                    {
                        if (0.5 < fmod(i, 2) && fmod(i, 2) <= 1.5) {
                            f_last + (f_m - f_m_last) <= -U_z ? f = f_last + (U_z - U_p) + (f_m - f_m_last) : f = f_last + (f_m - f_m_last);
                        }
                        else
                            f_last + (f_m - f_m_last) >= U_z ? f = f_last - (U_z - U_p) + (f_m - f_m_last) : f = f_last + (f_m - f_m_last);
                    }

                    f_m_last = f_m;
                    f_last = f;

                    fout << i << '\t' << f_m << '\t' << f << '\n';
                }

                fout << '\n';

            }
        }
    }

    fout.close();

    return 0;
}