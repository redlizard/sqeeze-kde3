/*
  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.
*/

/*
  Copyright (C) 2005 İsmail Dönmez <ismail@kde.org>
*/

#include "blowfish.h"
#include "mc_blowfish.h"
#include "server.h"
#include "channel.h"

#include <qcstring.h>
#include <qstringlist.h>


namespace Konversation
{
    // Find n'th occurrence of separator in input and return the index
    int findOccurrence(const QCString& input, const QCString& separator, int nth)
    {
        int j=1;
        uint i;

        for(i=0; i < input.length(); ++i)
        {
            if((input.mid(i,1) == separator))
            {
                if (j == nth)
                    return i;
                else
                    ++j;
            }
        }
        return i;
    }

    void decrypt(const QString& recipient, QCString& cipher, Server* server)
    {
        QCString key = server->getKeyForRecipient(recipient);

        if(!key.isEmpty())
        {
            int index = findOccurrence(cipher, ":", 2);
            QCString backup = cipher.mid(0,index+1);
            QCString tmp = cipher.mid(index+1);
            char* tmp2;

            if(server->identifyMsgEnabled()) // Workaround braindead Freenode prefixing messages with +
                tmp = tmp.mid(1);

            if( !(tmp.mid(0,4) == "+OK ") && !(tmp.mid(0,5) == "mcps ") )
                return;
            else
                cipher = tmp;

            if(cipher.mid(0,5) == "mcps ")
                cipher = cipher.mid(5);
            else
                cipher = cipher.mid(4);

            QCString ckey( key.length()+2 );
            QCString result( cipher.length()+1 );
            qstrncpy(result.data(), cipher.data(), cipher.length());
            qstrncpy(ckey.data(), key.data(), key.length()+1);
            tmp2 = decrypt_string(ckey.data(),result.data());
            const char *pfx="(e) ";
            // If it's a CTCP we don't want to have the (e) interfering with the processing
            if (tmp2[0] == 1)
                pfx = "\x0";
            cipher = backup+pfx+tmp2+' '+'\n'; // FIXME(??) why is there an added space here?
            free(tmp2);
        }
    }

    void decryptTopic(const QString& recipient, QCString& cipher, Server* server)
    {
        QCString key = server->getKeyForRecipient(recipient);

        if(!key.isEmpty())
        {
            int index = findOccurrence(cipher, ":", 2);
            QCString backup = cipher.mid(0,index+1);
            QCString tmp = cipher.mid(index+1);
            char* tmp2;

            if(tmp.mid(0,4) == "+OK ")            // FiSH style topic
                cipher = tmp.mid(4);
            else if(tmp.left(5) == "«m«")
                cipher = tmp.mid(5,tmp.length()-10);
            else
                return;

            QCString result( cipher.length()+1 );
            QCString ckey( key.length()+2 );
            qstrncpy(ckey.data(), key.data(), key.length()+1);
            qstrncpy(result.data(), cipher.data(), cipher.length());
            tmp2 = decrypt_string(ckey.data(),result.data());
            cipher = tmp2;
            if(cipher.mid(0,2) == "@@")
                cipher = cipher.mid(2);

            cipher = backup+"(e) "+cipher;
            free(tmp2);
        }
    }

    bool encrypt(const QString& key, QCString& cipher)
    {
        if(key.isEmpty())
            return false;

        if (cipher.left(3) == "+p ")
            cipher = cipher.mid(3);
        else
        {
            QCString ckey(key.local8Bit());

            char *tmp = encrypt_string(ckey.data(), cipher.data());
            cipher = QCString("+OK ") + tmp;
            free(tmp);
        }
        return true;
    }
}
