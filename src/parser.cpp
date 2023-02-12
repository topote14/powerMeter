#include "parser.h"

String item;
String trama = "";

String getFromTerminal()
{
  trama = "";
  trama = Serial.readString(); // leo todo
  Serial.println("1");
  StringSplitter *split = new StringSplitter(trama, '\n', 35); // separo para que lea hasta la fase S
  trama = split->getItemAtIndex(0);                            // guardo lo que separé hasta la "s"
  trama.remove(trama.length() - 2);                            // al final del string da cada fase guarda dos \t => se sacan

  // empiezo a separar por items
  StringSplitter *splitter = new StringSplitter(trama, '\t', 35); // new StringSplitter(string_to_split, delimiter, limit) Max 5 strings
  int itemCount = splitter->getItemCount();                       // Obtengo la cantidad de subStrings del mensaje entrante separados por un ";".

  Serial.println("itemCount: " + (String)itemCount);
  Serial.println("trama: " + trama);

  // reinicio el string largo de datos
  trama = "";
  for (int i = 0; i < itemCount; i++)
  {
    Serial.println("i: " + (String)i);

    item = splitter->getItemAtIndex(i); // Obtengo el String ya separado del buffer.

    if (item != "") // si parseo un string vacio
    {
      if (i != 29)
      {
        item = item + ','; // al ultimo valor le borro la coma porque hace bardo despues en el server
      }
      trama = trama + item;
    }
  }

  delete splitter; // libero la memoria asignada para splitter
  delete split;
  Serial.flush();
  // trama = "Fase[°],Vrms[V],Irms[A],Ipk[A],Imax[A],Ih1[A],Ih2[A],Ih3[A],Ih4[A],Ih5[A],Ih6[A],Ih7[A],Ithd[%],Pa[kW],E[kWh]\n"+trama;

  return (trama);
}
String parseData(String data)
{
  StringSplitter *split = new StringSplitter(data, '\n', 35); // separo para que lea hasta la fase S
  data = split->getItemAtIndex(0);                            // guardo lo que separé hasta la "s"
  data.remove(data.length() - 1);                             // al final del string da cada fase guarda dos \t => se sacan

  // empiezo a separar por items
  StringSplitter *splitter = new StringSplitter(data, '\t', 35); // new StringSplitter(string_to_split, delimiter, limit) Max 5 strings
  int itemCount = splitter->getItemCount();                      // Obtengo la cantidad de subStrings del mensaje entrante separados por un ";".


  // reinicio el string largo de datos
  data = "";
  for (int i = 0; i < itemCount; i++)
  {
    //Serial.println("i: " + (String)i);

    item = splitter->getItemAtIndex(i); // Obtengo el String ya separado del buffer.

    if (item != "") // si parseo un string vacio
    {
      if (i != 29)
      {
        item = item + ','; // al ultimo valor le borro la coma porque hace bardo despues en el server
      }
      data = data + item;
    }
  }

  delete splitter; // libero la memoria asignada para splitter
  delete split;
  Serial.flush();
  // data = "Fase[°],Vrms[V],Irms[A],Ipk[A],Imax[A],Ih1[A],Ih2[A],Ih3[A],Ih4[A],Ih5[A],Ih6[A],Ih7[A],Ithd[%],Pa[kW],E[kWh]\n"+trama;
  Serial.println("data.: " + data);
  return (data);
}