//
//  XpressNetLogFile.h
//  XpressNetQt
//
//  Created by Aiko Pras on 05-11-2026.
//  Copyright (c) 2026 by Aiko Pras. All rights reserved.
//
//  Qt version of the LogFileClass of the VB6 XpressNet DLL. See XpressNetLogFile.cpp
//
//  THE FOLLOWING METHODS ARE DEFINED:
//  OpenLogfile                        ' Check the Logfile size, and rotate the Logfiles if needed
//  Append          some Text          ' Append "some text" to the Logfile
//  AppendWithHost  some text          ' Append "some text", preceded by the letter of the XpressNet object
//  Connection      some text          ' 2.0: Append "= some text" (the connection of the XpressNet object)
//  SendHex         byteArray          ' Append "=>" followed by the hex values of byteArray
//  ReceiveHex      byteArray          ' Append "<=" followed by the hex values (without the last byte)
//  ReceiveHexRange byeArray, from, to ' Append "<=" followed by the hex values between from and to
//
//  Details         some Text          ' Same as AppendWithHost, but checks Parent.LogDetails first
//  DetailsSent     some Text          ' 2.0: as Details, preceded by "=>" (description of a sent message)
//  DetailsReceived some Text          ' 2.0: as Details, preceded by "<=" (description of a received message)
//  Errors          some Text          ' Same as AppendWithHost, but checks Parent.LogErrors first
//  Warnings        some Text          ' Same as AppendWithHost, but checks Parent.LogWarnings first
//
//  LogDirectory / setLogDirectory     ' The directory of the logfiles (.NET: the property LogDirectory)
//

#ifndef XPRESSNETLOGFILE_H
#define XPRESSNETLOGFILE_H

#include <QByteArray>
#include <QString>

class XpressNetClass;

class LogFileClass
{
public:
  explicit LogFileClass(XpressNetClass *parent);

  void OpenLogfile();
  void Append(const QString &data);
  void AppendWithHost(const QString &data);
  void Details(const QString &data);
  void DetailsSent(const QString &data);                  // 2.0
  void DetailsReceived(const QString &data);              // 2.0
  void Connection(const QString &data);                   // 2.0
  void Errors(const QString &data);
  void Warnings(const QString &data);
  void SendHex(const QByteArray &byteArray);
  void ReceiveHex(const QByteArray &byteArray);
  void ReceiveHexRange(const quint8 *byteArray, int msgStart, int msgEnd);

  // The directory for the logfiles. If the directory does not exist, it will be created.
  // If that fails, the previous directory is kept
  QString LogDirectory() const;
  void setLogDirectory(const QString &directory);

  QString logFileName;
  QString logFileNameOld1;
  QString logFileNameOld2;
  QString logFileNameOld3;
  QString myHost;                                         // Allows the Logfile Hex dumps to include the IP address / USB port

private:
  XpressNetClass *Parent;
  static const qint64 maxLogFileSize = 100000;           // Size is in bytes
  QString m_LogDir;

  void setFileNames();
  static QString timeText();
  void writeLine(const QString &text);
};

#endif // XPRESSNETLOGFILE_H
