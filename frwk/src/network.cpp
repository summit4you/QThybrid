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

#include "frwk.h"
#include "network.h"

CNetworkReply::CNetworkReply( QObject *parent, const QNetworkRequest &req, const QNetworkAccessManager::Operation op )
	: QNetworkReply( parent )
{
	// Setup the request
	setRequest( req );
	setUrl( req.url() );
	setOperation( op );
	QNetworkReply::open( QIODevice::ReadOnly | QIODevice::Unbuffered );

	// Bogus content
	m_lOffset = 0;
	m_content.clear();

	// Get the path to the file
	QByteArray path = req.url().path().toUtf8();

	str::t_string8 mime = "application/octet-stream";
	str::t_string8 full = 
		disk::WebPath< str::t_string8 >( "htm", str::t_string8( path.data(), path.length() ) );

	printf( "%s(%d) : RES : %s\n", __FILE__, __LINE__, full.c_str() );

	// Check for linked in resources
	CHmResources res;
	if ( res.IsValid() )
	{
		// See if there is such a resource
		HMRES hRes = res.FindResource( 0, full.c_str() );
		if ( hRes )
		{
			// Set return type
			setAttribute( QNetworkRequest::HttpStatusCodeAttribute, QVariant( 200 ) );

			switch ( res.Type( hRes ) )
			{
				default :
					break;

				case 1 :
				{
					mime = disk::GetMimeType( full );
					const void *ptr = res.Ptr( hRes );
					unsigned long sz = res.Size( hRes );

					if ( ptr && 0 < sz )
						m_content.append( QByteArray::fromRawData( (const char* )ptr, sz ) );

				} break;

				case 2 :
				{
					mime = "text/html";

					// Get function pointer
					CHmResources::t_fn pFn = res.Fn( hRes );
					if ( pFn )
					{
						// in / out
						TPropertyBag< str::t_string8 > in;
						TPropertyBag< str::t_string8 > out;
						
						// Copy GET parameters
						long szQi = req.url().encodedQueryItems().size();
						if ( szQi )
						{	TPropertyBag< str::t_string8 > &pbGet = in[ "GET" ];
							for( long i = 0; i < szQi; i++ ) 
							{	const QPair< QByteArray, QByteArray > it = req.url().encodedQueryItems().at( i );
								pbGet[ parser::DecodeUrlStr( str::t_string8( it.first.data(), it.first.length() ) ) ]
									= parser::DecodeUrlStr( str::t_string8( it.second.data(), it.second.length() ) );
							} // end for
						} // end if

						// Execute the page
						pFn( in, out );

						// Set the output
						m_content.append( out.data(), out.length() );

					} // end if

				} break;

			} // end switch

		} // end if

		else
			setAttribute( QNetworkRequest::HttpStatusCodeAttribute, QVariant( 404 ) );

	} // end if

	else
		setAttribute( QNetworkRequest::HttpStatusCodeAttribute, QVariant( 404 ) );

	// Data size
	setHeader( QNetworkRequest::ContentLengthHeader, QVariant( m_content.size() ) );

	// Access control
	// setRawHeader( "Access-Control-Allow-Origin", "*" );

	// MIME Type
	if ( mime.length() )
		setHeader( QNetworkRequest::ContentTypeHeader, QVariant( mime.c_str() ) );

	// Call notify functions
	QMetaObject::invokeMethod( this, "metaDataChanged", Qt::QueuedConnection );
	QMetaObject::invokeMethod( this, "readyRead", Qt::QueuedConnection );
	QMetaObject::invokeMethod( this, "downloadProgress", Qt::QueuedConnection,
							   Q_ARG( qint64, m_content.size() ), Q_ARG( qint64, m_content.size() ) );
	QMetaObject::invokeMethod( this, "finished", Qt::QueuedConnection );

}

qint64 CNetworkReply::readData( char* pData, qint64 lMaxSize )
{
	// Have we copied all the data?
	if ( m_lOffset >= m_content.size() )
		return -1;

	// Copy a block of data
	qint64 lCount = qMin( lMaxSize, m_content.size() - m_lOffset );
	memcpy( pData, m_content.constData() + m_lOffset, lCount );
	m_lOffset += lCount;

	// Return the number of bytes copied
	return lCount;
}

CNetworkMgr::CNetworkMgr( QObject *pParent, QNetworkAccessManager *pPrev )
	: QNetworkAccessManager( pParent )
{
	if ( pPrev )
	{
		setCache( pPrev->cache() );
		setCookieJar( pPrev->cookieJar() );
		setProxy( pPrev->proxy() );
		setProxyFactory( pPrev->proxyFactory() );

	} // end if

}

QNetworkReply* CNetworkMgr::createRequest( QNetworkAccessManager::Operation op, const QNetworkRequest &req, QIODevice *device )
{
	// printf( "%s(%d) : %s\n", __FILE__, __LINE__, req.url().toString().toUtf8().data() );

	if ( req.url().host() == "embedded" )
		return new CNetworkReply( this, req, op );

	return new CNetworkReply( this, req, op );
	
	// This could be enabled to allow network access
	// return QNetworkAccessManager::createRequest( op, req, device );
}
