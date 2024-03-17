#include <iostream>
#include <string>
#include <fstream>
#include <vector>
#include <sstream>
#include <cmath>
#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>

sf::Font font;
sf::Text text;

using namespace std;
using namespace sf;

float prex, prey, prez, ConX, ConY;
float RotX, RotY, RotZ;
float WY, zoom;
int XOff = 720, YOff = 540, ZOff, lap = 1, lapIndex;

int FocalLength = 2000;
int packetLength = 13;
string line;

float AvgSpeed[200], AvgThrottle[200];
string last[200], best[200];
int laps[200];

vector<float> d1;
vector<float> d2;
vector<float> d3;

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

RectangleShape convertToLengthAndRotation(double x1, double y1, double x2, double y2, int thicc, int i)
{
  CompleteRotation(d1[lapIndex], d3[lapIndex], d2[lapIndex]);
  float length = sqrtf(pow((x1 - x2), 2) + pow((y1 - y2), 2));
  float dir = atan2(y1 - y2, x1 - x2) * 180 / 3.141592653589793234338327950288;
  RectangleShape line;
  line.setPosition(Vector2f(x1 * zoom, y1 * zoom));
  line.setSize(Vector2f(length * zoom, thicc));
  line.rotate(dir);
  if (x1 == ConX + XOff && y1 == ConY + YOff)
  {
    line.setFillColor(Color(255, 255, 255));
    line.setSize(Vector2f(length * thicc * zoom, thicc));
  }
  else
  {
    line.setFillColor(Color(d2[i + 2], d1[i + 2], 0));
  }
  return line;
}

Text drawText(int x, int y, int size, string t, bool leftAllign)
{
  Text text;
  text.setFont(font);
  text.setString(t);
  text.setCharacterSize(size);
  FloatRect bounds = text.getLocalBounds();
  if (leftAllign)
    text.setPosition(Vector2f(x, y));
  else
    text.setPosition(Vector2f(x - bounds.width, y));
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

string convertToTimestamp()
{
  int millis = abs((int)floorf((lapIndex - laps[lap]) / packetLength * 16) % 1000);
  int sec = abs((int)floorf((lapIndex - laps[lap]) / (packetLength * 60)) % 60);
  string min = to_string(abs((int)floorf((lapIndex - laps[lap]) / (packetLength * 3600))));
  string temp1, temp2;
  if (millis < 100)
  {
    if (millis < 10)
    {
      temp1 = "00" + to_string(millis);
    }
    else
    {
      temp1 = "0" + to_string(millis);
    }
  }
  else
  {
    temp1 = to_string(millis);
  }
  if (sec < 10)
  {
    temp2 = "0" + to_string(sec);
  }
  else
  {
    temp2 = to_string(sec);
  }
  return min + ":" + temp2 + "." + temp1;
}

int main()
{
  //*********************READ FILE*********************//

  int DataLength = 0;
  ifstream Data("data.txt");
  int lineCount = 0;
  int i = 0;
  int i2 = 0;
  int lastLap = 0;
  bool change;
  while (getline(Data, line))
  {
    i = 0;
    i2 = 0;

    while (line[i] != ' ')
    {
      i++;
    }
    d1.push_back((float)stof(line.substr(1, i - 1).c_str()));
    if (lineCount % packetLength == 9 && lastLap != d3[lineCount - 7])
    {
      change = true;
      last[lastLap] = line.substr(1, i - 1);
    }

    i++;
    i2 = i;
    while (line[i] != ' ')
    {
      i++;
    }
    d2.push_back((float)stof(line.substr(i2, i - i2).c_str()));
    if (change)
    {
      best[lastLap] = line.substr(i2, i - i2);
      lastLap = d3[lineCount - 7];
    }
    change = false;

    i++;
    i2 = i;
    while (line[i] != '>')
    {
      i++;
    }
    d3.push_back((float)stof(line.substr(i2, i - i2).c_str()));
    lineCount++;
  }
  Data.close();

  //******************GET LAP INDEXES******************//

  DataLength = d1.size();
  i2 = 0;
  for (int i = 0; i < DataLength; i += packetLength)
  {
    if (i2 != d3[i + 2])
    {
      i2 = d3[i + 2];
      laps[i2] = i;
      cout << i2 << " " << laps[i2] << endl;
    }
  }
  lapIndex = laps[lap];

  //****************CALCULATE AVERAGES*****************//

  i = 1;
  float avg1, avg2;
  while (laps[i] != 0)
  {
    for (i2 = laps[i]; i2 < laps[i + 1]; i2 += packetLength)
    {
      avg1 += (d1[i2 + 1] * 3.6) / ((laps[i + 1] - laps[i]) / packetLength);
      avg2 += (d1[i2 + 2] / 2.55) / ((laps[i + 1] - laps[i]) / packetLength);
    }
    AvgSpeed[i] = avg1;
    AvgThrottle[i] = avg2;
    avg1 = 0;
    avg2 = 0;
    i++;
  }

  //****************START WINDOW RENDER****************//
  RenderWindow window(VideoMode(1920, 1080), "Gran Turismo Visulalizer");
  window.setVerticalSyncEnabled(true);
  window.setKeyRepeatEnabled(false);
  font.loadFromFile("Anta-Regular.ttf");
  bool keys[8]; // W, A, S, D, Up, Down, Left, Right
  bool replay;
  while (window.isOpen())
  {
    Event event;
    while (window.pollEvent(event))
    {
      if (event.type == Event::Closed)
        window.close();
      if (event.type == sf::Event::KeyPressed && clock() > 0.5)
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
    if (keys[0])
      YOff++;
    if (keys[1])
      XOff++;
    if (keys[2])
      YOff--;
    if (keys[3])
      XOff--;
    if (keys[4])
      RotX += 0.01;
    if (keys[5])
      RotX -= 0.01;
    if (keys[6])
      RotY += 0.01;
    if (keys[7])
      RotY -= 0.01;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Z))
      zoom += 0.1;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::X))
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
      CompleteRotation(d1[i], d3[i], d2[i]);
      float ConX2 = ConX;
      float ConY2 = ConY;
      CompleteRotation(d1[i + packetLength * 4], d3[i + packetLength * 4], d2[i + packetLength * 4]);
      window.draw(convertToLengthAndRotation(ConX2 + XOff, ConY2 + YOff, ConX + XOff, ConY + YOff, 5, i));
    }
    window.draw(drawText(10, 10, 24, "Lap: " + to_string(lap), 1));
    window.draw(drawText(10, 40, 24, "Current: " + convertToTimestamp(), 1));
    window.draw(drawText(10, 70, 24, "Last: " + last[lap - 1], 1));
    window.draw(drawText(10, 100, 24, "Best: " + best[lap - 1], 1));
    window.draw(drawText(10, 130, 24, "Avg Speed: " + to_string(AvgSpeed[lap]) + "km/h", 1));
    window.draw(drawText(10, 160, 24, "Avg Throttle: " + to_string(AvgThrottle[lap]) + "%", 1));

    window.draw(drawText(1920 - 220, 8, 24, "RPM", 1));
    window.draw(drawGauge(1920 - 220, 34, 200, d1[lapIndex + 8], d2[lapIndex + 1]));
    window.draw(drawText(1920 - 215, 34, 24, to_string((int)d2[lapIndex + 1]), 1));

    window.draw(drawText(1920 - 440, 8, 24, "km/h", 1));
    window.draw(drawGauge(1920 - 440, 34, 200, d2[lapIndex + 8], d1[lapIndex + 1] * 3.6));
    window.draw(drawText(1920 - 435, 34, 24, to_string((int)(d1[lapIndex + 1] * 3.6)), 1));

    window.draw(drawText(1920 - 220, 68, 24, "kPa", 1));
    window.draw(drawGauge(1920 - 220, 94, 200, 6, d1[lapIndex + 7]));
    window.draw(drawText(1920 - 215, 94, 24, to_string(d1[lapIndex + 7]), 1));

    window.draw(drawText(1920 - 440, 68, 24, "Bar", 1));
    window.draw(drawGauge(1920 - 440, 94, 200, 8, d2[lapIndex + 7]));
    window.draw(drawText(1920 - 435, 94, 24, to_string(d2[lapIndex + 7]), 1));

    window.draw(drawText(1800, 8, 24, "Gear: " + to_string((int) d3[lapIndex + 1]), 1));

    window.draw(drawSuspension(1000, 30, 80, d1[lapIndex + 3]));
    window.draw(drawWheel(1000, d3[lapIndex + 5]));
    window.draw(drawText(975, WY - 15, 24, to_string((int)d3[lapIndex + 5]) + "C", 0));
    window.draw(drawSuspension(1050, 30, 80, d2[lapIndex + 3]));
    window.draw(drawWheel(1050, d1[lapIndex + 6]));
    window.draw(drawText(1065, WY - 15, 24, to_string((int)d1[lapIndex + 6]) + "C", 1));
    window.draw(drawSuspension(1000, 100, 80, d3[lapIndex + 3]));
    window.draw(drawWheel(1000, d2[lapIndex + 6]));
    window.draw(drawText(975, WY - 15, 24, to_string((int)d2[lapIndex + 6]) + "C", 0));
    window.draw(drawSuspension(1050, 100, 80, d1[lapIndex + 4]));
    window.draw(drawWheel(1050, d3[lapIndex + 6]));
    window.draw(drawText(1065, WY - 15, 24, to_string((int)d3[lapIndex + 6]) + "C", 1));

    window.draw(drawPedalInput(30, 400, d1[lapIndex + 2], Color(0, 255, 0)));
    window.draw(drawPedalInput(50, 400, d2[lapIndex + 2], Color(255, 0, 0)));
    window.draw(drawPedalInput(70, 400, d3[lapIndex + 8], Color(0, 0, 255)));

    window.display();
    if (replay)
    {
      lapIndex += packetLength;
      if (lapIndex > laps[lap + 1])
      {
        if (laps[lap] != 0)
        {
          lap++;
        }
      }
    }
  }
  return 0;
}