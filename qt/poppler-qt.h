/* poppler-qt.h: qt interface to poppler
 * Copyright (C) 2005, Net Integration Technologies, Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#ifndef __POPPLER_QT_H__
#define __POPPLER_QT_H__

#include <qcstring.h>
#include <qdatetime.h>
#include <qpixmap.h>

namespace Poppler {

class PageData;
class Page {
  friend class Document;
  public:
    ~Page();
    void renderToPixmap(QPixmap **q, int x, int y, int w, int h) const;
  private:
    Page(const Document *doc, int index);
    PageData *data;
};

class DocumentData;

class Document {
  friend class Page;
  
public:
  enum PageMode {
    UseNone,
    UseOutlines,
    UseThumbs,
    FullScreen,
    UseOC
  };
  
  static Document *Document::load(const QString & filePath);
  
  Page *getPage(int index) const{ return new Page(this, index); }
  
  int getNumPages() const;
  
  PageMode getPageMode() const;
  
  bool unlock(QCString &password);
  
  bool isLocked() const;
  
  QDateTime getDate( const QString & data ) const;
  QString getInfo( const QString & data ) const;
  bool isEncrypted() const;
  bool isLinearized() const;
  bool okToPrint() const;
  bool okToChange() const;
  bool okToCopy() const;
  bool okToAddNotes() const;
  double getPDFVersion() const;
  
  Document::~Document();
  
private:
  DocumentData *data;
  Document::Document(DocumentData *dataA);
};

}
#endif
