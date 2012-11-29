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
#	include "network.h"
#	include "web_page.h"
#endif

#include <QtNetwork/QNetworkReply>
#include <QMainWindow>
#include <QPointer>
#include "network.h"
#include "web_page.h"
#include "htmapp.h"

class CMainWindow : public QMainWindow
{
	Q_OBJECT

public:

	/// Constructor
	CMainWindow();

	/// Initializes the window
	void Init();

	/// Set the project name
	void setName( str::t_string8 s ) { m_name = s; }

	/// Set the project description
	void setDescription( str::t_string8 s ) { m_desc = s; }

	/// Set the initial url
	void setHomeUrl( str::t_string8 s ) { m_url = s; }

	/// Set the window size
	void setWindowSize( long w, long h ) { m_width = w; m_height = h; }

public slots:

	// On network finished handler
	void onFinished( QNetworkReply *reply );

private:

	/// Web view
	QPointer< QWebView > 		m_pView;

	/// Web page
	QPointer< CWebPage > 		m_pPage;

	/// Custom network object
	QPointer< CNetworkMgr > 	m_pNet;

	/// Initial URL
	str::t_string8				m_url;

	/// Project name
	str::t_string8				m_name;

	/// Project Description
	str::t_string8				m_desc;

	/// Initial window width
	long						m_width;

	/// Initial window height
	long						m_height;

};
