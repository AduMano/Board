#include "locker.h"

Locker::Locker(String lockerSize)
{
  size = lockerSize;
  isOpen = true; // Default to open
  userOwner = "";
}

void Locker::openLocker()
{
  isOpen = true;
  Serial.println("Locker opened.");
}

void Locker::closeLocker()
{
  isOpen = false;
  Serial.println("Locker closed.");
}

void Locker::updateStatus(bool open, String ownerId)
{
  isOpen = open;
  userOwner = ownerId;
}
