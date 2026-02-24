#include <iostream> 
#include <string>
#include <list> 
#include "../include/tokenizador.h"

using namespace std;

void imprimirListaSTL(const list<string>& cadena)
{
        list<string>::const_iterator itCadena;
        for(itCadena=cadena.begin();itCadena!=cadena.end();itCadena++)
        {
                cout << (*itCadena) << ", ";
        }
        cout << endl;
}

int
main(void)
{
list<string> lt1, lt2;
Tokenizador b("@.,&", true, false);
b.Tokenizar("...10.000.a.000 ,,23.05 10/12/85 1,23E+10 .21", lt2);

imprimirListaSTL(lt2);

}
