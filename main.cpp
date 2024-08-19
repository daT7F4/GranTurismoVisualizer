#include <iostream>
#include <string>
#include <fstream>
#include <vector>
#include <sstream>
#include <cmath>
#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>
#include <SFML/System.hpp>

sf::Font font;
sf::Text text;
sf::Clock fps;
sf::Texture timeBack;

using namespace std;
using namespace sf;

float prex, prey, prez, ConX, ConY, x;
float RotX, RotY = -90, RotZ;
float WY, zoom = 0.5;
int XOff = 0, YOff = 0, ZOff, lap = 1;
float delta;
uint64_t lapIndex, singleLapIndex;

int FocalLength = 2000;
int packetLength = 15;
string line;

float AvgSpeed[200], AvgThrottle[200];
string last[200], best[200];
int laps[200];

vector<float> d1;
vector<float> d2;
vector<float> d3;

bool displaySetting = true;

int InfoX = 5, InfoY = 5;
int CarDataX = 1435, CarDataY = 5;
int WheelDataX = 1620, WheelDataY = 140; // 1615 - 1435 = 180
int PedalDataX = 1825, PedalDataY = 140;
int AssistX = 1435, AssistY = 140;
bool infoT, carT, wheelT, pedalT, seekT, AssistT, TrackT;
int relativeX, relativeY;

bool mouseState = false;

double PI = 3.141592653589793234338327950288;

void rotateX(float x, float y, float z, float XRot)
{
  prex = x;
  prey = (y * cos(XRot)) - (z * sin(XRot));
  prez = (y * sin(XRot)) + (z * cos(XRot));
}

void rotateY(float x, float y, float z, float YRot)
{
  prex = (x * cos(YRot)) + (z * sin(YRot));
  prey = y;
  prez = ((0 - x) * sin(YRot)) + (z * cos(YRot));
}

void rotateZ(float x, float y, float z, float ZRot)
{
  prex = (x * sin(ZRot)) - (y * sin(ZRot));
  prey = (x * sin(ZRot)) + (y * cos(ZRot));
  prez = z;
}

void ConvertDimensions(float x, float y, float z, int fl)
{
  ConX = (fl * (x / 10)) / (fl + (z / 10));
  ConY = (fl * (y / 10)) / (fl + (z / 10));
}

void CompleteRotation(float x, float y, float z)
{
  rotateX(x, y, z, RotX);
  rotateY(prex, prey, prez, RotY);
  rotateZ(prex, prey, prez, RotZ);
  ConvertDimensions(prez, prey, prez, FocalLength);
}

void CompleteRotation(float x, float y, float z, float RX, float RY, float RZ, int FL)
{
  rotateX(x, y, z, RX);
  rotateY(prex, prey, prez, RY);
  rotateZ(prex, prey, prez, RZ);
  ConvertDimensions(prez, prey, prez, FL);
}

RectangleShape drawTrack(double x1, double y1, double x2, double y2, int thicc, int i, float speed, float max, float rot)
{
  CompleteRotation(d1[lapIndex], d3[lapIndex], d2[lapIndex] * 5);
  float length = sqrtf(pow((x1 - x2), 2) + pow((y1 - y2), 2));
  float dir = atan2(y1 - y2, x1 - x2) * 180 / PI;
  RectangleShape line;
  line.setPosition(Vector2f(x1 * zoom + 960, y1 * zoom + 540));
  line.setSize(Vector2f(length * zoom, thicc));
  if (displaySetting)
    line.setFillColor(Color(d2[i + 2], d1[i + 2], 0));
  else
    line.setFillColor(Color(speed / max * 255, 255 - (speed / max * 255), 0));

  if (x1 == ConX + XOff && y1 == ConY + YOff)
  {
    line.setFillColor(Color(255, 255, 255));
    line.setSize(Vector2f(2 * zoom, thicc));
    line.rotate((rot + 1) * -180 - 90);
  }
  else
  {
    line.rotate(dir);
  }
  return line;
}

Text drawText(int x, int y, int size, string t, Color color, bool centerAllign)
{
  Text text;
  text.setFont(font);
  text.setString(t);
  text.setCharacterSize(size);
  text.setFillColor(color);
  FloatRect bounds = text.getLocalBounds();
  if (centerAllign)
    text.setPosition(Vector2f(x, y));
  else
    text.setPosition(Vector2f(x - (bounds.width / 2), y));
  return text;
}

RectangleShape drawGauge(int x, int y, float length, float max, float val)
{
  RectangleShape gauge;
  gauge.setPosition(Vector2f(x, y));
  gauge.setSize(Vector2f(val / max * length, 30));
  gauge.setFillColor(Color(val / max * 255, 255 - (val / max * 255), 0));
  return gauge;
}

RectangleShape drawSuspension(int x, int y, float max, float height)
{
  RectangleShape wheel;
  wheel.setPosition(Vector2f(x, y));
  wheel.setSize(Vector2f(max * height, 10));
  WY = max * height + y;
  wheel.setFillColor(Color(255, 255, 255));
  wheel.rotate(90);
  return wheel;
}

CircleShape drawWheel(int x, int temperatue)
{
  CircleShape wheel(15);
  wheel.setPosition(Vector2f(x - 20, WY - 15));
  wheel.setFillColor(sf::Color(255, 255 - temperatue, 255 - temperatue));
  return wheel;
}

RectangleShape drawPedalInput(int x, int y, int val, Color color)
{
  RectangleShape pedal;
  pedal.setPosition(Vector2f(x, y));
  pedal.setSize(Vector2f(val / 2, 20));
  pedal.setFillColor(color);
  pedal.rotate(-90);
  return pedal;
}

RectangleShape drawRect(int x, int y, int w, int h, Color color)
{
  RectangleShape rect;
  rect.setPosition(Vector2f(x, y));
  rect.setSize(Vector2f(w, h));
  rect.setFillColor(color);
  return rect;
}

CircleShape drawCircle(int x, int y, int r, Color color)
{
  CircleShape circle(r);
  circle.setPosition(Vector2f(x, y));
  circle.setFillColor(color);
  return circle;
}

string convertToTimestamp()
{
  int millis = abs((int)floorf((lapIndex - laps[lap]) / packetLength) % 60 * 16.6666666666);
  int sec = abs((int)floorf((lapIndex - laps[lap]) / packetLength) % 3600 / 60);
  string min = to_string(abs((int)floorf((lapIndex - laps[lap]) / (packetLength * 3600))));
  string temp1, temp2;
  if (millis < 100)
  {
    if (millis < 10)
      temp1 = "00" + to_string(millis);
    else
      temp1 = "0" + to_string(millis);
  }
  else
    temp1 = to_string(millis);
  if (sec < 10)
    temp2 = "0" + to_string(sec);
  else
    temp2 = to_string(sec);
  return min + ":" + temp2 + "." + temp1;
}

RectangleShape background(int x, int y, int w, int h)
{
  RectangleShape ground;
  ground.setPosition(Vector2f(x, y));
  ground.setSize(Vector2f(w, h));
  ground.setFillColor(Color(50, 50, 50, 150));
  return ground;
}

float TyreDistanceRelativity(float radius, float rps, float speed)
{
  return 2 * PI * radius * rps;
}

void lapCheck()
{
  if (lapIndex > laps[lap + 1])
  {
    lap++;
  }
  else if (lapIndex < laps[lap])
  {
    lap--;
  }
}

float limit(float x){
  if(x > 255){
    return 255;
  } else{
    return x;
  }
}

string average(float value, int decimals) {
    ostringstream out;
    out << fixed << setprecision(decimals) << value;
    return out.str();
}

int main()
{

  //*********************READ FILE*********************//

  int DataLength = 0;
  ifstream Data("data.txt");
  int i = 0;
  int i2 = -1;
  int lastLap = 0;
  bool change;
  string la;
  while (getline(Data, line))
  {
    string input = line.substr(1, line.size() - 2);
    if((i % packetLength) == 9 && line != la){
      i2++;
      la = line;
      last[i2] = line.substr(1, 10);
      best[i2] = line.substr(12, 10);
      cout << i << " " << last[i2] << " " << best[i2] << endl;
    }
    size_t pos;
    for (auto &c : input)
    {
      if (!isdigit(c) && c != '.' && c != '-' && c != ':')
      {
        c = ' ';
      }
    }
    istringstream iss(input);
    double x, y, z;
    iss >> x >> y >> z;

    d1.push_back(x);
    d2.push_back(y);
    d3.push_back(z);
    i++;
  }
  Data.close();

  //******************GET LAP INDEXES******************//

  DataLength = d1.size();
  i2 = 1;
  for (int i = 0; i < DataLength; i += packetLength)
  {
    if (i2 != d3[i + 2])
    {
      i2 = d3[i + 2];
      laps[i2] = i;
    }
  }
  DataLength /= packetLength;
  lapIndex = laps[lap];
  laps[i2 + 1] = 1;

  //****************CALCULATE AVERAGES*****************//

  i = 0;
  float avg1, avg2;
  while (laps[i] != 1)
  {
    for (i2 = laps[i]; i2 < laps[i + 1]; i2 += packetLength)
    {
      avg1 += (d1[i2 + 1] * 3.6) / ((laps[i + 1] - laps[i]) / packetLength);
      avg2 += (d1[i2 + 2] / 2.55) / ((laps[i + 1] - laps[i]) / packetLength);
    }
    if (avg1 < 2000.0)
      AvgSpeed[i] = avg1;
    AvgThrottle[i] = avg2;
    avg1 = 0;
    avg2 = 0;
    i++;
  }

  //****************START WINDOW RENDER****************//
  Time elapsed1 = fps.getElapsedTime();
  RenderWindow window(VideoMode(1920, 1080), "Gran Turismo Visulalizer");
  window.setFramerateLimit(120);
  window.setKeyRepeatEnabled(false);
  font.loadFromFile("Silkscreen-Regular.ttf");
  bool keys[8]; // W, A, S, D, Up, Down, Left, Right
  bool replay;
  while (window.isOpen())
  {
    Event event;
    while (window.pollEvent(event))
    {
      if (event.type == Event::Closed)
        window.close();
      if (event.type == sf::Event::MouseWheelScrolled)
        zoom += event.mouseWheelScroll.delta;
      if (event.type == sf::Event::MouseButtonPressed)
        mouseState = true;
      if (event.type == sf::Event::MouseButtonReleased)
        mouseState = false;
      if (event.type == sf::Event::KeyPressed)
      {
        if (event.key.code == sf::Keyboard::W)
          keys[0] = true;
        if (event.key.code == sf::Keyboard::A)
          keys[1] = true;
        if (event.key.code == sf::Keyboard::S)
          keys[2] = true;
        if (event.key.code == sf::Keyboard::D)
          keys[3] = true;
        if (event.key.code == sf::Keyboard::Up)
          keys[4] = true;
        if (event.key.code == sf::Keyboard::Down)
          keys[5] = true;
        if (event.key.code == sf::Keyboard::Left)
          keys[6] = true;
        if (event.key.code == sf::Keyboard::Right)
          keys[7] = true;
        if (event.key.code == sf::Keyboard::Space)
          replay = !replay;
        if (event.key.code == sf::Keyboard::Escape)
          window.close();
        if (event.key.code == sf::Keyboard::Num1)
        {
          if (lap != 0)
          {
            lap--;
            lapIndex = laps[lap];
          }
        }
        if (event.key.code == sf::Keyboard::Num2)
        {
          if (lap != d3[lapIndex + 7])
          {
            lap++;
            lapIndex = laps[lap];
          }
        }
        if (event.key.code == sf::Keyboard::C)
          displaySetting = !displaySetting;
      }
      if (event.type == sf::Event::KeyReleased)
      {
        if (event.key.code == sf::Keyboard::W)
          keys[0] = false;
        if (event.key.code == sf::Keyboard::A)
          keys[1] = false;
        if (event.key.code == sf::Keyboard::S)
          keys[2] = false;
        if (event.key.code == sf::Keyboard::D)
          keys[3] = false;
        if (event.key.code == sf::Keyboard::Up)
          keys[4] = false;
        if (event.key.code == sf::Keyboard::Down)
          keys[5] = false;
        if (event.key.code == sf::Keyboard::Left)
          keys[6] = false;
        if (event.key.code == sf::Keyboard::Right)
          keys[7] = false;
      }
    }
    float offset = 1.5;
    if (keys[0])
      YOff += offset;
    if (keys[1])
      XOff += offset;
    if (keys[2])
      YOff -= offset;
    if (keys[3])
      XOff -= offset;
    if (keys[4])
      RotX += 0.01;
    if (keys[5])
      RotX -= 0.01;
    if (keys[6])
      RotY += 0.01;
    if (keys[7])
      RotY -= 0.01;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Z))
      if (zoom < 100.0)
        zoom += 0.1;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::X))
      if (zoom > 0)
        zoom -= 0.1;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::R))
    {
      XOff = 0;
      YOff = 0;
      ZOff = 0;
      RotX = 0;
      RotY = 0;
      RotZ = 0;
    }

    window.clear();
    for (int i = laps[lap]; i < laps[lap + 1] - packetLength * 2; i += packetLength)
    {
      CompleteRotation(d1[i], d3[i], d2[i] * 5);
      float ConX2 = ConX;
      float ConY2 = ConY;
      CompleteRotation(d1[i + packetLength * 4], d3[i + packetLength * 4], d2[i + packetLength * 4] * 5);
      window.draw(drawTrack(ConX2 + XOff, ConY2 + YOff, ConX + XOff, ConY + YOff, 5, i, d1[i + 1] * 3.6, d2[i + 8], d2[i + 11]));
    }
    window.draw(background(InfoX, InfoY, 370, 185));

    window.draw(drawText(InfoX + 5, InfoY, 24, "Lap: " + to_string(lap) + " of " + to_string((int)d3[((DataLength - 1) * packetLength) + 2]), Color(255, 255, 255), 1));
    window.draw(drawText(InfoX + 5, InfoY + 30, 24, "Current: " + convertToTimestamp(), Color(255, 255, 255), 1));
    if (last[lap - 1] != "00:00.001 " && lap != 0)
      window.draw(drawText(InfoX + 5, InfoY + 60, 24, "Last: " + last[lap - 1], Color(255, 255, 255), 1));
    else
      window.draw(drawText(InfoX + 5, InfoY + 60, 24, "Last: --:--.---", Color(255, 255, 255), 1));
    if (best[lap - 1] != "00:00.001 " && lap != 0)
      window.draw(drawText(InfoX + 5, InfoY + 90, 24, "Best: " + best[lap - 1], Color(255, 255, 255), 1));
    else
      window.draw(drawText(InfoX + 5, InfoY + 90, 24, "Best: --:--.---", Color(255, 255, 255), 1));
    window.draw(drawText(InfoX + 5, InfoY + 120, 24, "Avg Speed: " + average(AvgSpeed[lap], 2) + "km/h", Color(255, 255, 255), 1));
    window.draw(drawText(InfoX + 5, InfoY + 150, 24, "Avg Throttle: " + average(AvgThrottle[lap], 2) + "%", Color(255, 255, 255), 1));

    window.draw(background(CarDataX, CarDataY, 480, 130));

    window.draw(drawText(CarDataX + 227, CarDataY + 3, 24, "RPM", Color(255, 255, 255), 1));
    window.draw(drawRect(CarDataX + 230, CarDataY + 29, 200, 30, Color(50, 50, 50)));
    window.draw(drawGauge(CarDataX + 230, CarDataY + 29, 200, d1[lapIndex + 8], d2[lapIndex + 1]));
    window.draw(drawText(CarDataX + 235, CarDataY + 29, 24, to_string((int)d2[lapIndex + 1]), Color(255, 255, 255), 1));

    window.draw(drawText(CarDataX + 7, CarDataY + 3, 24, "km/h", Color(255, 255, 255), 1));
    window.draw(drawRect(CarDataX + 10, CarDataY + 29, 200, 30, Color(50, 50, 50)));
    window.draw(drawGauge(CarDataX + 10, CarDataY + 29, 200, d2[lapIndex + 8], d1[lapIndex + 1] * 3.6));
    window.draw(drawText(CarDataX + 15, CarDataY + 29, 24, to_string((int)(d1[lapIndex + 1] * 3.6)), Color(255, 255, 255), 1));

    window.draw(drawText(CarDataX + 227, CarDataY + 63, 24, "Boost Pressure", Color(255, 255, 255), 1));
    window.draw(drawRect(CarDataX + 230, CarDataY + 89, 200, 30, Color(50, 50, 50)));
    window.draw(drawGauge(CarDataX + 230, CarDataY + 89, 200, 6, d1[lapIndex + 7]));
    window.draw(drawText(CarDataX + 235, CarDataY + 89, 24, average(d1[lapIndex + 7] - 1, 2), Color(255, 255, 255), 1));
    window.draw(drawText(CarDataX + 325, CarDataY + 89, 24, "Atm", Color(255, 255, 255), 1));

    window.draw(drawText(CarDataX + 7, CarDataY + 63, 24, "Oil Pressure", Color(255, 255, 255), 1));
    window.draw(drawRect(CarDataX + 10, CarDataY + 89, 200, 30, Color(50, 50, 50)));
    window.draw(drawGauge(CarDataX + 10, CarDataY + 89, 200, 8, d2[lapIndex + 7]));
    window.draw(drawText(CarDataX + 15, CarDataY + 89, 24, average(d2[lapIndex + 7], 2), Color(255, 255, 255), 1));
    window.draw(drawText(CarDataX + 100, CarDataY + 89, 24, "Atm", Color(255, 255, 255), 1));

    if (d3[lapIndex + 1] > 0 && d3[lapIndex + 1] < 8)
      window.draw(drawText(CarDataX + 330, CarDataY + 3, 24, "Gear: " + to_string((int)d3[lapIndex + 1]), Color(255, 255, 255), 1));
    else if(d3[lapIndex + 1] == 0)
      window.draw(drawText(CarDataX + 330, CarDataY + 3, 24, "Gear: R", Color(255, 255, 255), 1));
    else 
      window.draw(drawText(CarDataX + 330, CarDataY + 3, 24, "Gear: N", Color(255, 255, 255), 1));

    window.draw(background(WheelDataX, WheelDataY, 200, 200)); // 140

    float tyreSlip = TyreDistanceRelativity(d1[lapIndex + 13], d2[lapIndex + 4], d1[lapIndex + 1]);
    window.draw(drawRect(WheelDataX + 10, WheelDataY + 15, 80, 80, Color(255, 0, 0, limit(tyreSlip * 25))));
    window.draw(drawSuspension(WheelDataX + 55, WheelDataY + 50, 80, d1[lapIndex + 3]));
    window.draw(drawWheel(WheelDataX + 55, d3[lapIndex + 5]));
    window.draw(drawText(WheelDataX + 48, WheelDataY + 31, 16, to_string((int)abs(d2[lapIndex + 4])) + "RPS", Color(255, 255, 255), 0));
    window.draw(drawText(WheelDataX + 48, WheelDataY + 18, 16, to_string((int)d3[lapIndex + 5]) + "C", Color(255, 255, 255), 0));

    tyreSlip = TyreDistanceRelativity(d2[lapIndex + 13], d3[lapIndex + 4], d1[lapIndex + 1]);
    window.draw(drawRect(WheelDataX + 110, WheelDataY + 15, 80, 80, Color(255, 0, 0, limit(tyreSlip * 25))));
    window.draw(drawSuspension(WheelDataX + 155, WheelDataY + 50, 80, d2[lapIndex + 3]));
    window.draw(drawWheel(WheelDataX + 155, d1[lapIndex + 6]));
    window.draw(drawText(WheelDataX + 148, WheelDataY + 31, 16, to_string((int)abs(d3[lapIndex + 4])) + "RPS", Color(255, 255, 255), 0));
    window.draw(drawText(WheelDataX + 148, WheelDataY + 18, 16, to_string((int)d1[lapIndex + 6]) + "C", Color(255, 255, 255), 0));

    tyreSlip = TyreDistanceRelativity(d3[lapIndex + 13], d1[lapIndex + 5], d1[lapIndex + 1]);
    window.draw(drawRect(WheelDataX + 10, WheelDataY + 105, 80, 80, Color(255, 0, 0, limit(tyreSlip * 25))));
    window.draw(drawSuspension(WheelDataX + 55, WheelDataY + 140, 80, d3[lapIndex + 3]));
    window.draw(drawWheel(WheelDataX + 55, d2[lapIndex + 6]));
    window.draw(drawText(WheelDataX + 48, WheelDataY + 121, 16, to_string((int)abs(d1[lapIndex + 5])) + "RPS", Color(255, 255, 255), 0));
    window.draw(drawText(WheelDataX + 48, WheelDataY + 108, 16, to_string((int)d2[lapIndex + 6]) + "C", Color(255, 255, 255), 0));

    tyreSlip = TyreDistanceRelativity(d1[lapIndex + 14], d2[lapIndex + 5], d1[lapIndex + 1]);
    window.draw(drawRect(WheelDataX + 110, WheelDataY + 105, 80, 80, Color(255, 0, 0, limit(tyreSlip * 25))));
    window.draw(drawSuspension(WheelDataX + 155, WheelDataY + 140, 80, d1[lapIndex + 4]));
    window.draw(drawWheel(WheelDataX + 155, d3[lapIndex + 6]));
    window.draw(drawText(WheelDataX + 148, WheelDataY + 121, 16, to_string((int)abs(d2[lapIndex + 5])) + "RPS", Color(255, 255, 255), 0));
    window.draw(drawText(WheelDataX + 148, WheelDataY + 108, 16, to_string((int)d3[lapIndex + 6]) + "C", Color(255, 255, 255), 0));

    window.draw(background(PedalDataX, PedalDataY, 90, 147));

    window.draw(drawPedalInput(PedalDataX + 10, PedalDataY + 137, d1[lapIndex + 2], Color(0, 255, 0)));
    window.draw(drawPedalInput(PedalDataX + 30, PedalDataY + 137, d2[lapIndex + 2], Color(255, 0, 0)));
    window.draw(drawPedalInput(PedalDataX + 50, PedalDataY + 137, d3[lapIndex + 8], Color(0, 0, 255)));

    window.draw(drawText(10, 920, 16, "Sample " + to_string((int)singleLapIndex) + " of " + to_string((int)DataLength), Color(255, 255, 255), 1));
    window.draw(background(10, 900, 1900, 20));
    for(int i = 0; i < d3[((DataLength - 1) * packetLength) + 2]; i++){
      x = (((float)laps[i] / (float)(DataLength * packetLength)) * 1900) + 10;
      window.draw(drawRect(x, 900, 3, 20, Color(180, 180, 180)));
    }
    x = (((float)singleLapIndex / (float)DataLength) * 1900) + 10;
    window.draw(drawRect(x, 900, 3, 20, Color(255, 0, 0)));

    window.draw(background(AssistX, AssistY, 180, 60));
    window.draw(drawCircle(AssistX + 10, AssistY + 10, 20, Color(255, 255 - (d1[lapIndex + 10] * 255), 255 - (d1[lapIndex + 10] * 255))));
    window.draw(drawText(AssistX + 28, AssistY + 20, 16, "HB", Color(0, 0, 0), 0));
    window.draw(drawCircle(AssistX + 70, AssistY + 10, 20, Color(255, 255 - (d2[lapIndex + 10] * 255), 255 - (d2[lapIndex + 10] * 255))));
    window.draw(drawText(AssistX + 88, AssistY + 20, 16, "TCS", Color(0, 0, 0), 0));
    window.draw(drawCircle(AssistX + 130, AssistY + 10, 20, Color(255, 255 - (d3[lapIndex + 10] * 255), 255 - (d3[lapIndex + 10] * 255))));
    window.draw(drawText(AssistX + 148, AssistY + 20, 16, "ASM", Color(0, 0, 0), 0));

    // window.draw(drawText(1600, 280, 24, to_string((int)d3[lapIndex + 14]) + "L/" + to_string((int)d2[lapIndex + 14]) + "L", Color(255, 255, 255), 0));

    Time elapsed1 = fps.restart();
    window.draw(drawText(0, 935, 24, to_string((int)(1 / elapsed1.asSeconds())) + " FPS", Color(255, 255, 255), 1));

    window.display();
    if (replay)
    {
      delta += 60 / (1 / elapsed1.asSeconds());
      singleLapIndex = floor(delta);
      lapIndex = singleLapIndex * packetLength;
      lapCheck();
    }
    Vector2i pixelPos = sf::Mouse::getPosition(window);
    if (pixelPos.x > InfoX && pixelPos.x < InfoX + 370 && pixelPos.y > InfoY && pixelPos.y < InfoY + 185 || infoT)
    {
      if (mouseState)
      {
        if (!infoT)
        {
          relativeX = pixelPos.x - InfoX;
          relativeY = pixelPos.y - InfoY;
        }
        InfoX = pixelPos.x - relativeX;
        InfoY = pixelPos.y - relativeY;
        infoT = true;
      }
      else
      {
        infoT = false;
      }
    } else if (pixelPos.x > CarDataX && pixelPos.x < CarDataX + 440 && pixelPos.y > CarDataY && pixelPos.y < CarDataY + 130 || carT)
    {
      if (mouseState)
      {
        if (!carT)
        {
          relativeX = pixelPos.x - CarDataX;
          relativeY = pixelPos.y - CarDataY;
        }
        CarDataX = pixelPos.x - relativeX;
        CarDataY = pixelPos.y - relativeY;
        carT = true;
      }
      else
      {
        carT = false;
      }
    } else if (pixelPos.x > WheelDataX && pixelPos.x < WheelDataX + 140 && pixelPos.y > WheelDataY && pixelPos.y < WheelDataY + 200 || wheelT)
    {
      if (mouseState)
      {
        if (!wheelT)
        {
          relativeX = pixelPos.x - WheelDataX;
          relativeY = pixelPos.y - WheelDataY;
        }
        WheelDataX = pixelPos.x - relativeX;
        WheelDataY = pixelPos.y - relativeY;
        wheelT = true;
      }
      else
      {
        wheelT = false;
      }
    } else if (pixelPos.x > PedalDataX && pixelPos.x < PedalDataX + 90 && pixelPos.y > PedalDataY && pixelPos.y < PedalDataY + 147 || pedalT)
    {
      if (mouseState)
      {
        if (!pedalT)
        {
          relativeX = pixelPos.x - PedalDataX;
          relativeY = pixelPos.y - PedalDataY;
        }
        PedalDataX = pixelPos.x - relativeX;
        PedalDataY = pixelPos.y - relativeY;
        pedalT = true;
      }
      else
      {
        pedalT = false;
      }
    } else if (pixelPos.x > x && pixelPos.x < (x + 20) && pixelPos.y > 900 && pixelPos.y < 920 || seekT)
    {
      if (mouseState && pixelPos.x > 10 && pixelPos.x < 1900)
      {
        seekT = true;
        singleLapIndex = (DataLength / 1900) * (pixelPos.x - 10);
        delta = singleLapIndex;
        lapIndex = singleLapIndex * packetLength;
        lapCheck();
      }
      else
      {
        seekT = false;
      }
    } else if (pixelPos.x > AssistX && pixelPos.x < AssistX + 180 && pixelPos.y > AssistY && pixelPos.y < AssistY + 60 || AssistT)
    {
      if (mouseState)
      {
        if (!AssistT)
        {
          relativeX = pixelPos.x - AssistX;
          relativeY = pixelPos.y - AssistY;
        }
        AssistX = pixelPos.x - relativeX;
        AssistY = pixelPos.y - relativeY;
        AssistT = true;
      }
      else
      {
        AssistT = false;
      }
    } else{
      if(mouseState){
        if(!TrackT){
          relativeX = (pixelPos.x / zoom) - XOff;
          relativeY = (pixelPos.y / zoom) - YOff;
        }
        XOff = (pixelPos.x / zoom) - relativeX;
        YOff = (pixelPos.y / zoom) - relativeY;
        TrackT = true;
      } else{ 
        TrackT = false;
      }
    }
  }
  return 0;
}
