/****************************************************************************
**
** colordialog.h, derived from
** qcolordialog.h   2.3.2   edited 2001-01-26 Josua Dietze
** Copyright (C) 1992-2000 Trolltech AS.  All rights reserved.
**
*****************************************************************************/
#ifndef COLORDIALOG_H
#define COLORDIALOG_H

#include <qvalidator.h>
#include <qdialog.h>
#include <qframe.h>
#include <qlineedit.h>
#include <qlabel.h>
#include <qsizepolicy.h>

class ColorDialogPrivate;
class ColorShowLabel;
class QSizePolicy;
class ColorDialog : public QDialog
{
    Q_OBJECT
public:
    static QColor getColor( QColor, QWidget *parent=0, const char* name=0 );
    ColorDialog( QWidget* parent=0, const char* name=0, bool modal=FALSE );
    ~ColorDialog() {};
private:



    void setColor( QColor ); // ### 3.0: make const QColor&
    QColor color() const;
private:
    void setSelectedAlpha( int );
    int selectedAlpha() const;

    void showCustom( bool=TRUE );
private:
    ColorDialogPrivate *d;
    friend class ColorDialogPrivate;

};

class ColorPicker : public QFrame
{
    Q_OBJECT
public:
    ColorPicker(QWidget* parent=0, const char* name=0);
    ~ColorPicker();

public slots:
    void setCol( int h, int s );

signals:
    void newCol( int h, int s );

protected:
    QSize sizeHint() const;
    void drawContents(QPainter* p);
    void mouseMoveEvent( QMouseEvent * );
    void mousePressEvent( QMouseEvent * );

private:
    int hue;
    int sat;

    QPoint colPt();
    int huePt( const QPoint &pt );
    int satPt( const QPoint &pt );
    void setCol( const QPoint &pt );

    QPixmap *pix;
};

class ColorLuminancePicker : public QWidget
{
    Q_OBJECT
public:
    ColorLuminancePicker(QWidget* parent=0, const char* name=0);
    ~ColorLuminancePicker();

public slots:
    void setCol( int h, int s, int v );
    void setCol( int h, int s );

signals:
    void newHsv( int h, int s, int v );

protected:
//    QSize sizeHint() const;
//    QSizePolicy sizePolicy() const;
    void paintEvent( QPaintEvent*);
    void mouseMoveEvent( QMouseEvent * );
    void mousePressEvent( QMouseEvent * );

private:
    enum { foff = 3, coff = 4 }; //frame and contents offset
    int val;
    int hue;
    int sat;

    int y2val( int y );
    int val2y( int val );
    void setVal( int v );

    QPixmap *pix;
};

class QColNumLineEdit : public QLineEdit
{
    Q_OBJECT
public:
    QColNumLineEdit( QWidget *parent, const char* name = 0 )
    : QLineEdit( parent, name ) { setMaxLength( 3 );}
    QSize sizeHint() const {
        return QSize( 30, //#####
        QLineEdit::sizeHint().height() ); }
    void setNum( int i ) {
        QString s;
        s.setNum(i);
        bool block = signalsBlocked();
        blockSignals(TRUE);
        setText( s );
        blockSignals(block);
    }
    int val() const { return text().toInt(); }
};

class ColorShower : public QWidget
{
    Q_OBJECT
public:
    ColorShower( QWidget *parent, const char *name = 0 );

    //things that don't emit signals
    void setHsv( int h, int s, int v );

    int currentAlpha() const { return alphaEd->val(); }
    void setCurrentAlpha( int a ) { alphaEd->setNum( a ); }
    void showAlpha( bool b );


    QRgb currentColor() const { return curCol; }

public slots:
    void setRgb( QRgb rgb );

signals:
    void newCol( QRgb rgb );
private slots:
    void rgbEd();
    void hsvEd();
private:
    void showCurrentColor();
    int hue, sat, val;
    QRgb curCol;
    QColNumLineEdit *hEd;
    QColNumLineEdit *sEd;
    QColNumLineEdit *vEd;
    QColNumLineEdit *rEd;
    QColNumLineEdit *gEd;
    QColNumLineEdit *bEd;
    QColNumLineEdit *alphaEd;
    QLabel *alphaLab;
    ColorShowLabel *lab;
    bool rgbOriginal;
};

class ColorShowLabel : public QFrame
{
    Q_OBJECT
public:
    ColorShowLabel( QWidget *parent ) :QFrame( parent )
    {
        setFrameStyle( QFrame::Panel|QFrame::Sunken );
        setBackgroundMode( PaletteBackground );
        setAcceptDrops( TRUE );
        mousePressed = FALSE;
    }
    void setColor( QColor c ) { col = c; }

signals:
    void colorDropped( QRgb );

protected:
    void drawContents( QPainter *p );

private:
    QColor col;
    bool mousePressed;
    QPoint pressPos;
};

class QColIntValidator: public QIntValidator
{
public:
    QColIntValidator( int bottom, int top,
           QWidget * parent, const char *name = 0 )
    :QIntValidator( bottom, top, parent, name ) {}

    QValidator::State validate( QString &, int & ) const;
};

class ColorDialogPrivate : public QObject
{
Q_OBJECT
public:
    ColorDialogPrivate( ColorDialog *p );
    QRgb currentColor() const { return cs->currentColor(); }
    void setCurrentColor( QRgb rgb );

    int currentAlpha() const { return cs->currentAlpha(); }
    void setCurrentAlpha( int a ) { cs->setCurrentAlpha( a ); }
    void showAlpha( bool b ) { cs->showAlpha( b ); }

private slots:
    void newHsv( int h, int s, int v );
    void newColorTypedIn( QRgb rgb );
private:
    ColorPicker *cp;
    ColorLuminancePicker *lp;
    ColorShower *cs;
    int nextCust;
};

#endif // COLORDIALOG_H
