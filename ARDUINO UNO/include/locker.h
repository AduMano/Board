#ifndef LOCKER_H
#define LOCKER_H

#include <Arduino.h>

class Locker
{
public:
  String size;
  bool isOpen;
  String userOwner;

  Locker(String lockerSize);
  void openLocker();
  void closeLocker();
  void updateStatus(bool open, String ownerId);
};

#endif
