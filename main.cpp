#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <cmath>
#include <omp.h>
#include <stdlib.h>
#include <algorithm>
#include <cstdio>
#include <chrono>

using namespace std;

// Estructura para almacenar un dato de accidente
struct AccidentData {
  //std::string date; // Fecha del accidente (formato ISO 8601)
  string date; // Fecha del accidente (formato ISO 8601)
  int count; // Cantidad de accidentes
};

// Función para leer los datos históricos del archivo .csv
std::vector<AccidentData> readData(const std::string& fileName) {
  std::vector<AccidentData> data;

  // Abrir el archivo .csv en modo lectura
  std::ifstream file(fileName);

  // Leer las líneas del archivo .csv una a una
  std::string line;
  int linea=0;
  while (std::getline(file, line)) {
    // Dividir la línea en campos separados por coma

    //if (linea > 0) cout << line << endl;

    std::stringstream lineStream(line);
    std::string cell;
    std::vector<std::string> cells;
    while (std::getline(lineStream, cell, ';')) {

      cells.push_back(cell);
    }

    // Almacenar el dato de accidente

    std::string s = cells[0];
    std::string chars = "-/.";
    for (char c: chars) {
        s.erase(std::remove(s.begin(), s.end(), c), s.end());
    }
    // Linea mayor que 0 para que no tome el encabezado del archivo .csv
    if (linea>0){
        AccidentData d;
        //cout << s << endl;
        d.date = s;
        d.count = std::stoi(cells[1]);
        data.push_back(d);
    }
    //AccidentData d;
    //d.date = cells[0];
    //d.count = std::stoi(cells[1]);
    //data.push_back(d);
    linea++;


  }

  return data;
}

// Función para ajustar un modelo de regresión lineal a los datos históricos
void fitLinearModel(const std::vector<AccidentData>& d, double& slope, double& intercept) {
  // Calcular la cantidad de datos
  int n = d.size();

  // Calcular la suma de las fechas y la suma de los accidentes
  double sumDates = 0, sumCounts = 0;
  #pragma omp parallel for reduction(+:sumDates, sumCounts)
  for (int i = 0; i < n; i++) {
    //double date = d[i].date;
    double date = std::stod (d[i].date);
    sumDates += date;
    sumCounts += d[i].count;
    //cout << date << endl;
  }

  // Calcular la suma de los cuadrados de las fechas y la suma de los productos fechas-accidentes
  double sumDatesSquared = 0, sumDateCounts = 0;
  #pragma omp parallel for reduction(+:sumDatesSquared, sumDateCounts)
  for (int i = 0; i < n; i++) {
    double date = std::stod (d[i].date);
    sumDatesSquared += date * date;
    sumDateCounts += date * d[i].count;
  }

  // Calcular la pendiente y el intercepto
  slope = (n * sumDateCounts - sumDates * sumCounts) / (n * sumDatesSquared - sumDates * sumDates);
  intercept = (sumCounts - slope * sumDates) / n;
}

// Función para predecir la cantidad de accidentes esperados para una fecha futura
int predictAccidentCount(const std::string& date, const double& slope, const double& intercept) {
  // Convertir la fecha a un número

  std::string s = date;
  std::string chars = "-/.";
  for (char c: chars) {
        s.erase(std::remove(s.begin(), s.end(), c), s.end());
    }

  int dateInt = std::stoi(s);

  // Evaluar la fórmula del modelo de regresión lineal
  int count = slope * dateInt + intercept;

  return count;
}


int main(int argc, char* argv[])
{

// Verifica que se haya proporcionado un argumento en la línea de comandos
  if (argc != 2) {
    std::cerr << "Uso: " << argv[0] << " FECHA_ISO_8601" << std::endl;
    return 1;
  }


  // Parsea la fecha en formato ISO 8601 a un objeto tm
  std::string iso_date_string = argv[1];
  int year, month, day;
  std::sscanf(iso_date_string.c_str(), "%d-%d-%d", &year, &month, &day);


  // Verifica si la fecha es válida comparando los valores de los campos con los valores mínimos y máximos permitidos
  if (year < std::numeric_limits<int>::min() || year > std::numeric_limits<int>::max() ||
      month < 1 || month > 12 ||
      day < 1 || day > 31) {
    std::cerr << "Error: La fecha proporcionada es invalida" << std::endl;
    return 1;
  }


    // Almacenamos la fecha futura en una variable
    string futureDate = argv[1];
    //cout << futureDate << endl;

    // Abrimos el archivo .csv en modo lectura
    ifstream file("datos_examen.csv");
    if (!file.is_open())
    {
        cerr << "Error: no se pudo abrir el archivo datos_examen.csv" << endl;
        return 1;
    }

    std::vector<AccidentData> data = readData("datos_examen.csv");

    double slope, intercept;
    fitLinearModel(data, slope, intercept);

    // Pedir al usuario que ingrese la fecha futura para la cual se quiere predecir la cantidad de accidentes
    //std::cout << "Ingrese la fecha futura (en formato ISO 8601): ";
    //std::string date;
    //std::cin >> date;

     // Predecir la cantidad de accidentes esperados para la fecha futura ingresada
    int count = predictAccidentCount(futureDate, slope, intercept);

    // Mostrar el resultado en pantalla
    std::cout << "La cantidad de accidentes esperados para la fecha " << futureDate << " es: " << count << std::endl;

} //fin del programa
