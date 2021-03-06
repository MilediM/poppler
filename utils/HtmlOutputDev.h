//========================================================================
//
// HtmlOutputDev.h
//
// Copyright 1997 Derek B. Noonburg
//
// Changed 1999 by G.Ovtcharov
//========================================================================

//========================================================================
//
// Modified under the Poppler project - http://poppler.freedesktop.org
//
// All changes made under the Poppler project to this file are licensed
// under GPL version 2 or later
//
// Copyright (C) 2006, 2007, 2009, 2012 Albert Astals Cid <aacid@kde.org>
// Copyright (C) 2008, 2009 Warren Toomey <wkt@tuhs.org>
// Copyright (C) 2009, 2011 Carlos Garcia Campos <carlosgc@gnome.org>
// Copyright (C) 2009 Kovid Goyal <kovid@kovidgoyal.net>
// Copyright (C) 2010 Hib Eris <hib@hiberis.nl>
// Copyright (C) 2011 Joshua Richardson <jric@chegg.com>
// Copyright (C) 2011 Stephen Reichling <sreichling@chegg.com>
// Copyright (C) 2012 Igor Slepchin <igor.redhat@gmail.com>
// Copyright (C) 2012 Fabio D'Urso <fabiodurso@hotmail.it>
// Copyright (C) 2013 Thomas Freitag <Thomas.Freitag@alfa.de>
//
// To see a description of the changes please see the Changelog file that
// came with your tarball or type make ChangeLog if you are building from git
//
//========================================================================

#ifndef HTMLOUTPUTDEV_H
#define HTMLOUTPUTDEV_H

#ifdef __GNUC__
#pragma interface
#endif

#include <stdio.h>
#include "goo/gtypes.h"
#include "goo/GooList.h"
#include "GfxFont.h"
#include "OutputDev.h"
#include "HtmlLinks.h"
#include "HtmlFonts.h"
#include "Link.h"
#include "Catalog.h"
#include "UnicodeMap.h"


#ifdef _WIN32
#  define SLASH '\\'
#else
#  define SLASH '/'
#endif

#define xoutRound(x) ((int)(x + 0.5))

#define DOCTYPE "<!DOCTYPE html>"

class GfxState;
class GooString;
class PDFDoc;
class OutlineItem;
//------------------------------------------------------------------------
// HtmlString
//------------------------------------------------------------------------

enum UnicodeTextDirection {
  textDirUnknown,
  textDirLeftRight,
  textDirRightLeft,
  textDirTopBottom
};


class HtmlString {
public:

  // Constructor.
  HtmlString(GfxState *state, double fontSize, HtmlFontAccu* fonts);

  // Destructor.
  ~HtmlString();

  // Add a character to the string.
  void addChar(GfxState *state, double x, double y,
	       double dx, double dy,
	       Unicode u); 
  HtmlLink* getLink() { return link; }
  const HtmlFont &getFont() const { return *fonts->Get(fontpos); }
  void endString(); // postprocessing

private:
// aender die text variable
  HtmlLink *link;
  double xMin, xMax;		// bounding box x coordinates
  double yMin, yMax;		// bounding box y coordinates
  int col;			// starting column
  Unicode *text;		// the text
  double *xRight;		// right-hand x coord of each char
  HtmlString *yxNext;		// next string in y-major order
  HtmlString *xyNext;		// next string in x-major order
  int fontpos;
  GooString* htext;
  int len;			// length of text and xRight
  int size;			// size of text and xRight arrays
  UnicodeTextDirection dir;	// direction (left to right/right to left)
  HtmlFontAccu *fonts;
  
  friend class HtmlPage;

};


//------------------------------------------------------------------------
// HtmlPage
//------------------------------------------------------------------------



class HtmlPage {
public:

  // Constructor.
  HtmlPage(GBool rawOrder, char *imgExtVal);

  // Destructor.
  ~HtmlPage();

  // Begin a new string.
  void beginString(GfxState *state, GooString *s);

  // Add a character to the current string.
  void addChar(GfxState *state, double x, double y,
	       double dx, double dy, 
		double ox, double oy, 
		Unicode *u, int uLen); //Guchar c);

  void updateFont(GfxState *state);

  // End the current string, sorting it into the list of strings.
  void endString();

  // Coalesce strings that look like parts of the same line.
  void coalesce();

  // Find a string.  If <top> is true, starts looking at top of page;
  // otherwise starts looking at <xMin>,<yMin>.  If <bottom> is true,
  // stops looking at bottom of page; otherwise stops looking at
  // <xMax>,<yMax>.  If found, sets the text bounding rectange and
  // returns true; otherwise returns false.
  

  // new functions
  void AddLink(const HtmlLink& x){
    links->AddLink(x);
  }

  // add an image to the current page
  void addImage(GooString *fname, GfxState *state);

  // number of images on the current page
  int  getNumImages() { return imgList->getLength(); }

  void dump(FILE *f, int pageNum);

  // Clear the page.
  void clear();
  
  void conv();
private:
  HtmlFont* getFont(HtmlString *hStr) { return fonts->Get(hStr->fontpos); }

  double fontSize;		// current font size
  GBool rawOrder;		// keep strings in content stream order

  HtmlString *curStr;		// currently active string

  HtmlString *yxStrings;	// strings in y-major order
  HtmlString *xyStrings;	// strings in x-major order
  HtmlString *yxCur1, *yxCur2;	// cursors for yxStrings list
  
  void setDocName(char* fname);
  void dumpAsXML(FILE* f,int page);
  void dumpComplex(FILE* f, int page);
  int dumpComplexHeaders(FILE * const file, FILE *& pageFile, int page);

  // marks the position of the fonts that belong to current page (for noframes)
  int fontsPageMarker; 
  HtmlFontAccu *fonts;
  HtmlLinks *links; 
  GooList   *imgList;
  
  GooString *DocName;
  GooString *imgExt;
  int pageWidth;
  int pageHeight;
  int firstPage;                // used to begin the numeration of pages

  friend class HtmlOutputDev;
};

//------------------------------------------------------------------------
// HtmlMetaVar
//------------------------------------------------------------------------
class HtmlMetaVar {
public:
    HtmlMetaVar(const char *_name, const char *_content);
    ~HtmlMetaVar();    
    
    GooString* toString();	

private:

    GooString *name;
    GooString *content;
};

//------------------------------------------------------------------------
// HtmlOutputDev
//------------------------------------------------------------------------

class HtmlOutputDev: public OutputDev {
public:

  // Open a text output file.  If <fileName> is NULL, no file is written
  // (this is useful, e.g., for searching text).  If <useASCII7> is true,
  // text is converted to 7-bit ASCII; otherwise, text is converted to
  // 8-bit ISO Latin-1.  <useASCII7> should also be set for Japanese
  // (EUC-JP) text.  If <rawOrder> is true, the text is kept in content
  // stream order.
  HtmlOutputDev(Catalog *catalogA, char *fileName, char *title, 
	  char *author,
	  char *keywords,
	  char *subject,
	  char *date,
	  char *extension,
	  GBool rawOrder,
	  int firstPage = 1,
	  GBool outline = 0);

  // Destructor.
  virtual ~HtmlOutputDev();

  // Check if file was successfully created.
  virtual GBool isOk() { return ok; }

  //---- get info about output device

  // Does this device use upside-down coordinates?
  // (Upside-down means (0,0) is the top left corner of the page.)
  virtual GBool upsideDown() { return gTrue; }

  // Does this device use drawChar() or drawString()?
  virtual GBool useDrawChar() { return gTrue; }

  // Does this device use beginType3Char/endType3Char?  Otherwise,
  // text in Type 3 fonts will be drawn with drawChar/drawString.
  virtual GBool interpretType3Chars() { return gFalse; }

  // Does this device need non-text content?
  virtual GBool needNonText() { return gTrue; }

  //----- initialization and control

  virtual GBool checkPageSlice(Page *page, double hDPI, double vDPI,
                               int rotate, GBool useMediaBox, GBool crop,
                               int sliceX, int sliceY, int sliceW, int sliceH,
                               GBool printing,
                               GBool (* abortCheckCbk)(void *data) = NULL,
                               void * abortCheckCbkData = NULL,
                               GBool (*annotDisplayDecideCbk)(Annot *annot, void *user_data) = NULL,
                               void *annotDisplayDecideCbkData = NULL)
  {
   docPage = page;
   return gTrue;
  }


  // Start a page.
  virtual void startPage(int pageNum, GfxState *state, XRef *xref);

  // End a page.
  virtual void endPage();

  //----- update text state
  virtual void updateFont(GfxState *state);

  //----- text drawing
  virtual void beginString(GfxState *state, GooString *s);
  virtual void endString(GfxState *state);
  virtual void drawChar(GfxState *state, double x, double y,
			double dx, double dy,
			double originX, double originY,
			CharCode code, int nBytes, Unicode *u, int uLen);
  
  virtual void drawImageMask(GfxState *state, Object *ref, 
			     Stream *str,
			     int width, int height, GBool invert,
			     GBool interpolate, GBool inlineImg);
  virtual void drawImage(GfxState *state, Object *ref, Stream *str,
			 int width, int height, GfxImageColorMap *colorMap,
			 GBool interpolate, int *maskColors, GBool inlineImg);

  //new feature    
  virtual int DevType() {return 1234;}

  int getPageWidth() { return maxPageWidth; }
  int getPageHeight() { return maxPageHeight; }

  GBool dumpDocOutline(PDFDoc* doc);

private:
  // convert encoding into a HTML standard, or encoding->getCString if not
  // recognized. Will delete encoding for you and return a new one
  // that you have to delete
  static GooString* mapEncodingToHtml(GooString* encoding);
  void doProcessLink(AnnotLink *link);
  GooString* getLinkDest(AnnotLink *link);
  void dumpMetaVars(FILE *);
  void doFrame(int firstPage);
  GBool newHtmlOutlineLevel(FILE *output, GooList *outlines, Catalog* catalog, int level = 1);
  void newXmlOutlineLevel(FILE *output, GooList *outlines, Catalog* catalog);
#ifndef DISABLE_OUTLINE
  int getOutlinePageNum(OutlineItem *item);
#endif
  void drawJpegImage(GfxState *state, Stream *str);
  void drawPngImage(GfxState *state, Stream *str, int width, int height,
                    GfxImageColorMap *colorMap, GBool isMask = gFalse);
  GooString *createImageFileName(const char *ext);

  FILE *fContentsFrame;
  FILE *page;                   // html file
  //FILE *tin;                    // image log file
  //GBool write;
  GBool needClose;		// need to close the file?
  HtmlPage *pages;		// text for the current page
  GBool rawOrder;		// keep text in content stream order
  GBool doOutline;		// output document outline
  GBool ok;			// set up ok?
  GBool dumpJPEG;
  int pageNum;
  int maxPageWidth;
  int maxPageHeight;
  GooString *Docname;
  GooString *docTitle;
  GooList *glMetaVars;
  Catalog *catalog;
  Page *docPage;
  friend class HtmlPage;
};

#endif
