**Proste Odszumianie**

Prosta aplikacja do odszumiania sygnałów zapisanych w formacie .csv.

Aplikacja oferuje dwa algorytmy odszumiania:
  1. Średnia ruchoma (Moving average)
  2. Filter sinc

**Kompilacja - Linux**

W celu kompilacji aplikacji, należy przestrzegać poniższych kroków:
1. Zainstalowanie zależności biblioteki:
  ```
  sudo apt install cmake libfltk1.3-dev
  ```
2. Przejście do folderu ze źródłami programu
  ```
  cd /sciezka/do/projektu/
  ```
2. Zbudowanie Makefile:
  ```
  cmake .
  ```
3. Kompilacja:
  ```
  make -B
  ```
