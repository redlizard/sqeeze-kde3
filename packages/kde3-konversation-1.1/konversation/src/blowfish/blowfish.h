/*
  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.
*/

/*
  Copyright (C) 2005 İsmail Dönmez <ismail@kde.org>
*/

#ifndef BLOWFISH_H
#define BLOWFISH_H


class Server;

class QString;
class QCString;


namespace Konversation
{

    int findOccurrence(const QCString& input, const QCString& separator, int nth);
    void decrypt(const QString& recipient, QCString& cipher, Server* server);
    void decryptTopic(const QString& recipient, QCString& cipher, Server* server);
    bool encrypt(const QString& key, QCString& cipher);
}
#endif
