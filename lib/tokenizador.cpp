#include <iostream>
#include <sys/types.h>
#include <sys/stat.h>
#include <cstdlib>
#include <fstream>
#include "../include/tokenizador.h"
#include <string>
#include <unistd.h>
using namespace std;


Tokenizador::Tokenizador (const string& delimitadoresPalabra, const bool& kcasosEspeciales, const bool& minuscSinAcentos) {
    DelimitadoresPalabra(delimitadoresPalabra);
    casosEspeciales = kcasosEspeciales;
    pasarAminuscSinAcentos = minuscSinAcentos;
    ActualizarDelimitadoresEspeciales();
}

Tokenizador::Tokenizador (const Tokenizador& t) {
    delimiters = t.delimiters;
    casosEspeciales = t.casosEspeciales;
    pasarAminuscSinAcentos = t.pasarAminuscSinAcentos;
    ActualizarDelimitadoresEspeciales();
}

Tokenizador::Tokenizador () {
    delimiters = ",;:.-/+*\\ '\"{}[]()<>¡!¿?&#=\t@";
    casosEspeciales = true;
    pasarAminuscSinAcentos = false;
    ActualizarDelimitadoresEspeciales();
}

Tokenizador::~Tokenizador () {
    delimiters = "";
}   

Tokenizador& Tokenizador::operator= (const Tokenizador& t) {
    if (this != &t) {
        delimiters = t.delimiters;
        casosEspeciales = t.casosEspeciales;
        pasarAminuscSinAcentos = t.pasarAminuscSinAcentos;
        ActualizarDelimitadoresEspeciales();
    }
    return *this;
}

void Tokenizador::ActualizarDelimitadoresEspeciales() {

    const string excepcionesURL   = "_:/.?&-=#@";
    const string excepcionesNUM   = ".,";
    const string excepcionesEMAIL = ".-_";
    const string excepcionesACRON = ".";
    const string excepcionesGUION = "-";

    delimitersURL.clear();   delimitersNUM.clear();
    delimitersEMAIL.clear(); delimitersACRON.clear();
    delimitersGUION.clear();

    for (char c : delimiters) {
        if (excepcionesURL.find(c)   == string::npos) delimitersURL   += c;
        if (excepcionesNUM.find(c)   == string::npos) delimitersNUM   += c;
        if (excepcionesEMAIL.find(c) == string::npos) delimitersEMAIL += c;
        if (excepcionesACRON.find(c) == string::npos) delimitersACRON += c;
        if (excepcionesGUION.find(c) == string::npos) delimitersGUION += c;
    }

    esDelimPunto  = (delimiters.find('.')  != string::npos);
    esDelimComa   = (delimiters.find(',')  != string::npos);
    esDelimArroba = (delimiters.find('@')  != string::npos);
    esDelimGuion  = (delimiters.find('-')  != string::npos);

        // Tabla de normalización: índice=char ISO-8859-1, valor=char normalizado
    // Inicializar con tolower para todos
    for (int i = 0; i < 256; i++)
        tablaNorm[i] = (unsigned char)tolower((unsigned char)i);

    // Sobreescribir los acentuados
    // A
    for (unsigned char c : {192,193,194,195,196,197,224,225,226,227,228,229})
        tablaNorm[c] = 'a';
    // E
    for (unsigned char c : {200,201,202,203,232,233,234,235})
        tablaNorm[c] = 'e';
    // I
    for (unsigned char c : {204,205,206,207,236,237,238,239})
        tablaNorm[c] = 'i';
    // O
    for (unsigned char c : {210,211,212,213,214,242,243,244,245,246})
        tablaNorm[c] = 'o';
    // U
    for (unsigned char c : {217,218,219,220,249,250,251,252})
        tablaNorm[c] = 'u';
    // Ñ ñ
    tablaNorm[209] = tablaNorm[241] = 'n';
    // Ç ç
    tablaNorm[199] = tablaNorm[231] = 'c';
}

void Tokenizador::Tokenizar (const string& str, list<string>& tokens) const {

    string delimiters = this->delimiters; // copia local
    if (casosEspeciales) {
        if (delimiters.find(' ')  == string::npos) delimiters += ' ';
        if (delimiters.find('\n') == string::npos) delimiters += '\n';
        if (delimiters.find('\r') == string::npos) delimiters += '\r';
        if (delimiters.find('\t') == string::npos) delimiters += '\t';
    }


    tokens.clear();

    string txt = str;

    if (pasarAminuscSinAcentos) {
        txt = Normalizar(txt);
    }



    string::size_type lastPos = txt.find_first_not_of(delimiters,0);

    if(casosEspeciales)
    {
        bool esNumeroToken=true;
        while(string::npos != lastPos)
        {

            //URLs
            if (txt.compare(lastPos, 4, "ftp:") == 0 || txt.compare(lastPos, 5, "http:") == 0 || txt.compare(lastPos, 6, "https:") == 0) 
            {
                string::size_type fin = txt.find_first_of(delimitersURL, lastPos);
                if (fin == string::npos) {fin = txt.size();}
                if (txt[fin-1] !=':'){
                    string url = txt.substr(lastPos, fin - lastPos);
                    tokens.push_back(url);
                    lastPos = txt.find_first_not_of(delimiters, fin);
                }
                else
                {
                    string::size_type pos = txt.find_first_of(delimiters, lastPos);
                    tokens.push_back(txt.substr(lastPos, pos - lastPos));
                
                    // Avanzamos lastPos a la siguiente palabra
                    if (pos == string::npos){
                        lastPos = string::npos;
                    }
                    else{ 
                        lastPos = txt.find_first_not_of(delimiters, pos);
                    }
                }
                esNumeroToken = true;
            }
            //DECIMALES
            else if (esNumeroToken && (esDelimPunto || esDelimComa) && 
                (isdigit(txt[lastPos]) || ((txt[lastPos]=='.' || txt[lastPos]==',') 
                && lastPos+1<txt.length() && isdigit(txt[lastPos+1]))))
            {
                bool esNumero = true;
                string::size_type i = lastPos;


                while(i<txt.size())
                {
                    unsigned char c = txt[i];

                    if(isdigit(c) ||
                            (((c == '.' && esDelimPunto) ||
                            (c == ',' && esDelimComa)) && txt[i+1]!='.' && txt[i+1]!=','))
                    {
                        i++;
                    }
                    else if(delimiters.find(c)!=string::npos && c!='-')
                    {

                        break;
                    }
                    else
                    {
                        esNumero=false;
                        break;
                    }
                }

                if (esNumero){
                    string token = txt.substr(lastPos, i - lastPos);
                    if (txt[lastPos-1] == '.' || txt[lastPos-1] == ',')
                    {
                        token = "0" + std::string(1, txt[lastPos-1]) + token;
                    }

                    while (!token.empty() && (token.back()=='.' || token.back()==',')){
                    token.pop_back();
                    }

                    tokens.push_back(token);
                    lastPos = txt.find_first_not_of(delimiters, i);
                }
                else
                {
                    esNumeroToken = false;
                    continue; 
                }
            }//EMAILS
            else if(esDelimArroba && (txt.find('@', lastPos) <= txt.find_first_of(delimiters , lastPos)) &&  txt.find('@', lastPos) != lastPos && txt.find('@', lastPos) != string::npos)
            {
                string::size_type posArroba = txt.find('@', lastPos);

                string::size_type i = posArroba + 1;
                bool emailValido = true;
                // REGLA: Debe contener algo después de la @ y que no sea delimitador ni espacio
                bool tieneTextoDespues = (i < txt.size()  && 
                    delimiters.find(txt[i]) == string::npos);

                if(tieneTextoDespues)
                {
                    while(i<txt.size())
                    {
                        unsigned char c = txt[i];

                        if(c=='@')
                        {
                            emailValido=false;
                            break;
                        }

                        if (c == ' ' || c == '\t' || c == '\n' || c == '\r' || delimitersEMAIL.find(c) !=string::npos){
                            break;
                        }
                        else if(c == '.' || c=='-' || c=='_')
                        {
                            unsigned char prev = txt[i-1];
                            char next = (i + 1 < txt.size()) ? txt[i+1] : '\0';
                            if (delimiters.find(prev) == string::npos && prev != ' ' && prev != '.' && prev != '-' && prev != '_' &&
                                next != '\0' && delimiters.find(next) == string::npos && next != ' '){
                                i++;
                            }
                            else{ 
                                break;
                            }
                        }
                        else//Carácter normal
                        {
                            i++;
                        }


                    }
                    if (emailValido) {
                        tokens.push_back(txt.substr(lastPos, i - lastPos));
                        lastPos = txt.find_first_not_of(delimiters, i);
                    } else {
                        // Segundo @ encontrado 
                        if (posArroba > lastPos) {
                            tokens.push_back(txt.substr(lastPos, posArroba - lastPos));
                        }
                        lastPos = posArroba + 1;
                        continue;
                    }
                }
                else{//Fake email como marcos@
                    if (posArroba > lastPos){
                        tokens.push_back(txt.substr(lastPos, posArroba - lastPos));
                        lastPos = txt.find_first_not_of(delimiters, posArroba + 1);
                    }
                    else
                    {//solo @, saltar
                        lastPos = txt.find_first_not_of(delimiters, posArroba + 1);
                    }
                }
                esNumeroToken = true;
            }//ACRÓNIMOS
            else if(esDelimPunto && txt.find('.', lastPos) < txt.find_first_of(delimitersACRON + " \t\n\r", lastPos))
            {
                string::size_type i = lastPos;

                while(i<txt.size())
                {
                    unsigned char c = txt[i];
                    if (c == ' ' || c == '\t' || c == '\n' || c == '\r' || delimitersACRON.find(c) !=string::npos){
                        break;
                    }
                    else if(c == '.')
                    {
                        unsigned char prev = txt[i-1];
                        char next = (i + 1 < txt.size()) ? txt[i+1] : '\0';
                        if (delimiters.find(prev) == string::npos && prev != '.' &&
                            next != '\0' && delimiters.find(next) == string::npos && next != '.')
                            i++;
                        else break;
                    }
                    
                    else//Carácter normal
                    {
                        i++;
                    }

                }
                if (i > lastPos)
                    tokens.push_back(txt.substr(lastPos, i - lastPos));
    
                lastPos = (i >= txt.size()) ? string::npos : txt.find_first_not_of(delimiters, i);
                esNumeroToken = true;
            }//GUIONES
            else if (esDelimGuion &&
                txt.find('-', lastPos) < txt.find_first_of(delimitersGUION + " \t\n\r", lastPos) &&
                txt.find('-', lastPos) != lastPos && txt.find('-', lastPos) != string::npos)
            {
                string::size_type i = lastPos;

                while (i < txt.size())
                {
                    unsigned char c = txt[i];
                    if (c == ' ' || c == '\t' || c == '\n' || c == '\r') break;
                    if (c == '-')
                    {
                        char next = (i + 1 < txt.size()) ? txt[i+1] : '\0';
                        // Dos guiones seguidos o guion al final: cortar antes
                        if (next == '-' || next == '\0' || next == ' ' || delimiters.find(next) != string::npos) break;
                        i++;
                    }
                    else if (delimiters.find(c) != string::npos) {break;}
                    else {i++;}
                }

                if (i > lastPos)
                    tokens.push_back(txt.substr(lastPos, i - lastPos));

                lastPos = (i >= txt.size()) ? string::npos : txt.find_first_not_of(delimiters, i);

                esNumeroToken = true;
            }
            else
            {//NO es ningun caso especial
                string::size_type pos = txt.find_first_of(delimiters, lastPos);
                tokens.push_back(txt.substr(lastPos, pos - lastPos));
                
                // Avanzamos lastPos a la siguiente palabra
                if (pos == string::npos){
                    lastPos = string::npos;
                }
                else{ 
                    lastPos = txt.find_first_not_of(delimiters, pos);
                }

                esNumeroToken = true;
            }
            }
            
    }
    else
    {
        string::size_type pos = txt.find_first_of(delimiters,lastPos);

        while(string::npos != lastPos){
            tokens.push_back(txt.substr(lastPos, pos - lastPos));
            lastPos = txt.find_first_not_of(delimiters, pos);
            pos = txt.find_first_of(delimiters, lastPos);
        }
    }
}

string Tokenizador::Normalizar(const string& str) const {
    string resultado(str.size(), '\0');
    const unsigned char* src = reinterpret_cast<const unsigned char*>(str.data());
    unsigned char* dst = reinterpret_cast<unsigned char*>(&resultado[0]);
    for (size_t i = 0; i < str.size(); i++)
        dst[i] = tablaNorm[src[i]];
    return resultado;
}
bool Tokenizador::Tokenizar (const string& NomFichEntr, const string& NomFichSal) const {
    ifstream i(NomFichEntr, ios::binary | ios::ate);
    if (!i) {
        cerr << "ERROR: No existe el archivo: " << NomFichEntr << endl;
        return false;
    }

    // Leer fichero entero de golpe
    streamsize tam = i.tellg();
    i.seekg(0, ios::beg);
    string contenido(tam, '\0');
    if (!i.read(&contenido[0], tam)) {
        return false;
    }
    i.close();

    // Tokenizar en una sola pasada
    list<string> tokens;
    Tokenizar(contenido, tokens);

    // Construir salida en memoria y escribir de golpe
    ofstream f(NomFichSal, ios::binary);
    if (!f) {
        return false;
    }

    string salida;
    salida.reserve(tam); // estimación razonable del tamaño
    for (const string& t : tokens) {
        salida += t;
        salida += '\n';
    }
    f.write(salida.data(), salida.size());

    return true;
}

bool Tokenizador::Tokenizar(const string& i) const {
    // El enunciado dice: "guardando la salida en un fichero de nombre i 
    // añadiéndole extensión .tk (sin eliminar previamente la extensión de i)"
    string nombreSalida = i + ".tk";
    
    // Invocamos a la otra función miembro para reutilizar la lógica de apertura
    // de ficheros y manejo de errores.
    return Tokenizar(i, nombreSalida);
}

bool Tokenizador::TokenizarListaFicheros(const string& i) const {
    ifstream ficheroLista;
    string nombreFicheroIndividual;
    bool todoCorrecto = true;
    
    ficheroLista.open(i.c_str());
    if (!ficheroLista) {
        cerr << "ERROR: No existe el archivo: " << i << endl;
        return false;
    }

    while (getline(ficheroLista, nombreFicheroIndividual)) {
        if (!nombreFicheroIndividual.empty()) {
            // Comprobar si es un directorio -> saltarlo
            struct stat s;
            if (stat(nombreFicheroIndividual.c_str(), &s) == 0 && S_ISDIR(s.st_mode)) {
                continue; // es directorio, ignorar
            }
            if (!Tokenizar(nombreFicheroIndividual)) {
                todoCorrecto = false;
            }
        }
    }

    ficheroLista.close();
    return todoCorrecto;
}
bool Tokenizador::TokenizarDirectorio (const string& dirAIndexar) const {
    struct stat dir;
    // Compruebo la existencia del directorio
    int err=stat(dirAIndexar.c_str(), &dir);
    if(err==-1 || !S_ISDIR(dir.st_mode)){
        cerr << "ERROR: No existe el directorio: " << dirAIndexar << endl; // <--- OBLIGATORIO
        return false;
    }
    else {
        cout<<"Directorio";
        // Hago una lista en un fichero con find>fich
        string cmd="find "+dirAIndexar+" -follow |sort > .lista_fich";
        system(cmd.c_str());
        return TokenizarListaFicheros(".lista_fich");
    }
}

void Tokenizador::DelimitadoresPalabra(const string& nuevoDelimiters) {
    delimiters = "";
    for (char c : nuevoDelimiters) {
        if (delimiters.find(c) == string::npos) {
            delimiters += c;
        }
    }
    ActualizarDelimitadoresEspeciales();
}

void Tokenizador::AnyadirDelimitadoresPalabra(const string& nuevoDelimiters) {
    for (char c : nuevoDelimiters) {
        if (delimiters.find(c) == string::npos) {
            delimiters += c;
        }
    }
    ActualizarDelimitadoresEspeciales();
}

string Tokenizador::DelimitadoresPalabra() const
{
    return delimiters;
}

void Tokenizador::CasosEspeciales(const bool& nuevoCasosEspeciales)
{
    casosEspeciales=nuevoCasosEspeciales;
    ActualizarDelimitadoresEspeciales();
}

bool Tokenizador::CasosEspeciales()
{
    return casosEspeciales;
}

void Tokenizador::PasarAminuscSinAcentos(const bool& nuevoPasarAminuscSinAcentos)
{
    pasarAminuscSinAcentos=nuevoPasarAminuscSinAcentos;
}

bool Tokenizador::PasarAminuscSinAcentos()
{
    return pasarAminuscSinAcentos;
}


ostream& operator<<(ostream& os, const Tokenizador& t) {
    os << "DELIMITADORES: " << t.delimiters 
       << " TRATA CASOS ESPECIALES: " << t.casosEspeciales 
       << " PASAR A MINUSCULAS Y SIN ACENTOS: " << t.pasarAminuscSinAcentos;
    return os;
}