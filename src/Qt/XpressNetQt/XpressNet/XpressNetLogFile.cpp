//
//  XpressNetLogFile.cpp
//  XpressNetQt
//
//  Created by Aiko Pras on 05-11-2026.
//  Copyright (c) 2026 by Aiko Pras. All rights reserved.
//
//  Qt version of the LogFileClass of the VB6 XpressNet DLL. A port of XpressNetLogFile.cs of the .NET version.
//  Same logfile (TMC-LogFile.txt, with the backups TMC-LogFile-1..3.txt) and same text.
//  The logfile directory can be changed by the application program (setLogDirectory).
//  The default directory depends on the operating system, and is created by the library:
//  - Linux:   ~/TMC-Logfiles
//  - macOS:   ~/Library/Logs/TMC-Logfiles (as the macOS version of the library)
//  - Windows: %PUBLIC%\TMC-Logfiles (as the VB6 and .NET versions)
//  If the directory can not be created, the directory is kept anyway (the logfile can then not be written).
//  The lines end with LF (Windows: CR LF). The text is written as UTF-8.
//
//  2.0: every line of an XpressNet object starts with its letter (A, B, ...; see XpressNetClass::instanceId); when a
//  connection is made, a line "A = 192.168.0.200, port 5550, local port 54321" tells which connection the letter
//  belongs to. 1.3: every line ended with the IP address or COM port (myHost, which is still set)
//
//  Differences with the VB6 version (the same as the .NET version):
//  - If the logfile can not be written (for example: no access rights), the text is lost without an error
//    (VB6: run time error)
//

#include "XpressNetLogFile.h"
#include "XpressNet.h"
#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QFileInfo>


LogFileClass::LogFileClass(XpressNetClass *parent)
{
  Parent = parent;
  myHost = "";
  // In case the application doesn't provide a path, use the following defaults (see the header of this file)
#if defined(Q_OS_WIN)
  m_LogDir = qEnvironmentVariable("PUBLIC");              // Use the Windows Environ variable
  if (m_LogDir.isEmpty()) m_LogDir = "C:/Users/Public";   // Works on several systems, but may require admin rights
  m_LogDir = QDir(m_LogDir).filePath("TMC-Logfiles");
#elif defined(Q_OS_MACOS)
  m_LogDir = QDir(QDir::homePath()).filePath("Library/Logs/TMC-Logfiles");
#else
  m_LogDir = QDir(QDir::homePath()).filePath("TMC-Logfiles");
#endif
  // The directory is created if it does not exist yet. If that fails, the logfile can not be written; the text
  // is lost without an error
  QDir().mkpath(m_LogDir);
  setFileNames();
}

void LogFileClass::setFileNames()
{
  QDir dir(m_LogDir);
  logFileName = dir.filePath("TMC-LogFile.txt");
  logFileNameOld1 = dir.filePath("TMC-LogFile-1.txt");
  logFileNameOld2 = dir.filePath("TMC-LogFile-2.txt");
  logFileNameOld3 = dir.filePath("TMC-LogFile-3.txt");
}


void LogFileClass::OpenLogfile()
{
  // The goal of this subroutine is to check the size of the existing Logfile.
  // If that size exceeds maxLogFileSize, the Logfiles will be rotated and renamed.
  // If the logfiles can not be rotated, we continue with the existing logfile
  QFileInfo info(logFileName);
  if (info.exists() && (info.size() > maxLogFileSize)) {
    // Delete the oldest file and rename the others
    if (QFile::exists(logFileNameOld3)) QFile::remove(logFileNameOld3);
    if (QFile::exists(logFileNameOld2)) QFile::rename(logFileNameOld2, logFileNameOld3);
    if (QFile::exists(logFileNameOld1)) QFile::rename(logFileNameOld1, logFileNameOld2);
    QFile::rename(logFileName, logFileNameOld1);
  }
  // Write an empty line, to indicate a restart
  writeLine("");
}

void LogFileClass::Append(const QString &data)
{
  writeLine(timeText() + ": " + data);
}

void LogFileClass::Details(const QString &data)
{
  if (Parent->LogDetails) AppendWithHost(data);
}

// 2.0: the description of a message gets the same direction marker as its hex values
void LogFileClass::DetailsSent(const QString &data)
{
  Details("=> " + data);
}

void LogFileClass::DetailsReceived(const QString &data)
{
  Details("<= " + data);
}

void LogFileClass::Errors(const QString &data)
{
  if (Parent->LogErrors) AppendWithHost(data);
}

void LogFileClass::Warnings(const QString &data)
{
  if (Parent->LogWarnings) AppendWithHost(data);
}

void LogFileClass::AppendWithHost(const QString &data)
{
  // 2.0: preceded by the letter of the XpressNet object (1.3: followed by the host)
  // writeLine(timeText() + ": " + data + " (" + myHost + ")");
  writeLine(timeText() + ": " + Parent->instanceId + " " + data);
}

// 2.0: the connection of this XpressNet object
void LogFileClass::Connection(const QString &data)
{
  AppendWithHost("= " + data);
}

// The hex values are written as in the VB6 and .NET versions: without a leading zero (for example 5 instead of 05)
void LogFileClass::SendHex(const QByteArray &byteArray)
{
  // create a string with Hex values
  QString hexString;
  for (int i = 0; i < byteArray.size(); i++) {
    hexString.append(QString::number(quint8(byteArray[i]), 16).toUpper()).append(" ");
  }
  writeLine(timeText() + ": " + Parent->instanceId + " => " + hexString.trimmed());
}

void LogFileClass::ReceiveHexRange(const quint8 *byteArray, int msgStart, int msgEnd)
{
  // create a string with Hex values
  QString hexString;
  for (int i = msgStart; i <= msgEnd; i++) {
    hexString.append(QString::number(byteArray[i], 16).toUpper()).append(" ");
  }
  writeLine(timeText() + ": " + Parent->instanceId + " <= " + hexString.trimmed());
}

void LogFileClass::ReceiveHex(const QByteArray &byteArray)
{
  // create a string with Hex values (as in VB6: without the last byte)
  QString hexString;
  for (int i = 0; i < byteArray.size() - 1; i++) {
    hexString.append(QString::number(quint8(byteArray[i]), 16).toUpper()).append(" ");
  }
  writeLine(timeText() + ": " + Parent->instanceId + " <= " + hexString.trimmed());
}

QString LogFileClass::LogDirectory() const
{
  return m_LogDir;
}

void LogFileClass::setLogDirectory(const QString &directory)
{
  QString d = directory.trimmed();
  if (d.length() == 0) return;
  // remove a trailing (back)slash, but not the root directory
  if ((d.length() > 1) && (d.endsWith('/') || d.endsWith('\\'))) d.chop(1);
  // check folder exists. If not, create it. If that fails, keep the old m_LogDir and defaults
  if (!QDir(d).exists()) {
    if (!QDir().mkpath(d)) return;
  }
  // Once we are here, the folder already exists or has been created
  m_LogDir = d;
  setFileNames();
}


// ==================================================================================================================
// Internal use
QString LogFileClass::timeText()
{
  return QDateTime::currentDateTime().toString("yyyy/MM/dd HH:mm:ss.zzz");
}

void LogFileClass::writeLine(const QString &text)
{
  // QIODevice::Text: on Windows the line ends with CR LF, on Linux and macOS with LF
  QFile file(logFileName);
  if (file.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text)) {
    file.write((text + "\n").toUtf8());
    file.close();
  }
  // If the logfile could not be written, the text is lost
}
