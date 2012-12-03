/*------------------------------------------------------------------
// Copyright (c) 1997 - 2011
// Robert Umbehant
// htmapp@wheresjames.com
// http://www.wheresjames.com
//
// Redistribution and use in source and binary forms, with or
// without modification, are permitted for commercial and
// non-commercial purposes, provided that the following
// conditions are met:
//
// * Redistributions of source code must retain the above copyright
//   notice, this list of conditions and the following disclaimer.
// * The names of the developers or contributors may not be used to
//   endorse or promote products derived from this software without
//   specific prior written permission.
//
//   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND
//   CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,
//   INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
//   MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
//   DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
//   CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
//   SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
//   NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
//   LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
//   HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
//   CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
//   OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
//   EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//----------------------------------------------------------------*/

#pragma once

#if defined( Q_MOC_RUN ) || defined( HTM_MOC_RUN )
#	include "frwk.h"
#endif

#include "htmapp.h"
#include <QtNetwork/QNetworkReply>


class CNetworkReply : public QNetworkReply
{
	Q_OBJECT

public:

	/// Constructor
	CNetworkReply( QObject *parent, const QNetworkRequest &req, const QNetworkAccessManager::Operation op );

	/// Aborts the transfer
	void abort() { QNetworkReply::close(); }

	/// Return the number of bytes available
	qint64 bytesAvailable() const { return m_content.size(); }

	/// Return non zero for sequential data
	bool isSequential() const { return true; }

protected:

	/// Used to retrieve a block of content data
	qint64 readData( char* pData, qint64 lMaxSize );

private:

	/// Offset progress
	qint64 m_lOffset;

	/// Data buffer
	QByteArray m_content;

};

class CNetworkMgr : public QNetworkAccessManager
{
	Q_OBJECT

public:

	/// Constructor
	CNetworkMgr( QObject *pParent, QNetworkAccessManager *pPrev );



	TPropertyBag< str::t_string8 > m_post;

protected:

	/// Create a request response object
	QNetworkReply* createRequest( QNetworkAccessManager::Operation op, const QNetworkRequest &req, QIODevice *device );

};
