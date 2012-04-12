    /*

    Copyright (C) 2001,2002 Matthias Kretz
                            kretz@kde.org
                  2003      Arnold Krille
                            arnold@arnoldarts.de

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.

    */

#ifndef _KAUDIORECORDSTREAM__H
#define _KAUDIORECORDSTREAM__H

#include <qobject.h>

#include <kdelibs_export.h>

class KArtsServer;
namespace Arts { class StereoEffectStack; }

/**
 * @brief A KDE wrapper around Synth_AMAN_RECORD/AudioToByteStream/ByteSoundReceiver
 *
 * @author Matthias Kretz <kretz@kde.org>
 * @since 3.2
*/
class KDE_ARTS_EXPORT KAudioRecordStream : public QObject
{
	Q_OBJECT

	public:
		/**
		 * Construct a KAudioRecordStream
		 *
                 * @param server The server where it should record from.
                 * @param title The title that is shown in the AudioManager.
                 * @param parent You will propably want to pass the server as parent to so this stream gets deleted before the server disappears.
                 * @param name The name of the stream.
		 */
		KAudioRecordStream( KArtsServer * server, const QString & title, QObject * parent = 0, const char * name = 0 );

		~KAudioRecordStream();

		/**
		 * You can read @p size number of bytes. If blockingIO() is set the method
		 * will return as soon as it has all your data and could possibly block your
		 * program. The method returns the number of bytes that are actually in the
		 * buffer.
		 *
		 * @see usePolling()
		 */
		int read( char *, int size );

		/**
		 * If you set blockingIO to true the read method will wait
		 * until it has enough data to return exactly what you asked for. If
		 * blockingIO is false you can count on having control back to your
		 * program soon enough
		 */
		void setBlockingIO( bool );

		/**
		 * read blocking I/O setting
		 *
		 * @see setBlockingIO()
		 */
		bool blockingIO() const;

		/**
		 * If you want to poll for data using read() set this to true. If
		 * you'd rather not poll use the data() signal for asynchronous
		 * I/O. If you set polling to true and don't call read() or
		 * flush() the inputbuffer will grow endlessly.
		 *
		 * The default is false (if you want to use read() set this to true).
		 */
		void usePolling( bool );

		/**
		 * read whether polling is enabled
		 *
		 * @see usePolling()
		 */
		bool polling() const;

		/**
		 * @return The Effect Stack right after the Synth_AMAN_RECORD.
		 */
		Arts::StereoEffectStack effectStack() const;

		/**
		 * @return Wether it is running (recording) or not.
		 * @since 3.2
		 */
		bool running() const;

	public slots:
		/**
		 * Detaches the stream from the soundserver but doesn't remove the Synth_AMAN_RECORD
		 * so that the entry in the Audiomanager remains.
		 */
		void stop();

		/**
		 * Attach to the soundserver and start getting data to read. This method
		 * must be called as soon as you want to receive data. In any case you have
		 * to call start() before read()
		 *
		 * @param samplingRate  The sampling rate the stream should be resampled to. Use
		 *                      a number between 500 and 2000000.
		 * @param bits          The number of bits the stream should have. Only 8 and
		 *                      16 Bits are supported.
		 * @param channels      The number of channels (mono/stereo). Only 1 and 2 are
		 *                      supported.
		 */
		void start( int samplingRate, int bits, int channels );

		/**
		 * flush input buffer
		 */
		void flush();

	signals:
		/**
		 * Data from the aRts server has arrived. You should copy the data
		 * because the passed QByteArray will be deleted right after
		 * returning from your slot(s).
		 *
		 * @param data the data from the server
		 */
		void data( QByteArray & data );

		/**
		 * Wether this RecordStream is recording or not.
		 * @since 3.2
		 */
		void running( bool );

	private slots:
		void slotRestartedServer();
		void slotData( const char *, unsigned int );

	private:
		KAudioRecordStream( const KAudioRecordStream & );
		KAudioRecordStream & operator=( const KAudioRecordStream & );

		struct Data;
		Data * d;
};

#endif //_KAUDIORECORDSTREAM__H
