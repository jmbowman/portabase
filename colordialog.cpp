/****************************************************************************
**
** colordialog.cpp, derived from
** qcolordialog.cpp   2.3.2   edited 2001-03-01 Josua Dietze
** Copyright (C) 1999-2000 Trolltech AS.  All rights reserved.
**
**
**********************************************************************/

#include "colordialog.h"

#include <qwidget.h>
#include <qpainter.h>
#include <qlayout.h>
#include <qpushbutton.h>
#include <qimage.h>
#include <qpixmap.h>
#include <qdrawutil.h>
#include <qpe/qpeapplication.h>
#include "pbdialog.h"


static bool initrgb = FALSE;
static QRgb stdrgb[6*8];
static QRgb cusrgb[2*8];


static void initRGB()
{
    if ( initrgb )
	return;
    initrgb = TRUE;
    int i = 0;
    for ( int g = 0; g < 4; g++ )
	for ( int r = 0;  r < 4; r++ )
	    for ( int b = 0; b < 3; b++ )
		stdrgb[i++] = qRgb( r*255/3, g*255/3, b*255/2 );

    for ( i = 0; i < 2*8; i++ )
	cusrgb[i] = qRgb(0xff,0xff,0xff);
}


static inline void rgb2hsv( QRgb rgb, int&h, int&s, int&v )
{
    QColor c;
    c.setRgb( rgb );
    c.getHsv(h,s,v);
}

static int pWidth = 200;
static int pHeight = 200;



int ColorLuminancePicker::y2val( int y )
{
    int d = height() - 2*coff - 1;
    return 255 - (y - coff)*255/d;
}

int ColorLuminancePicker::val2y( int v )
{
    int d = height() - 2*coff - 1;
    return coff + (255-v)*d/255;
}

ColorLuminancePicker::ColorLuminancePicker(QWidget* parent,
						  const char* name)
    :QWidget( parent, name )
{
    hue = 100; val = 100; sat = 100;
    pix = 0;
    //    setBackgroundMode( NoBackground );
}

ColorLuminancePicker::~ColorLuminancePicker()
{
    delete pix;
}

void ColorLuminancePicker::mouseMoveEvent( QMouseEvent *m )
{
    setVal( y2val(m->y()) );
}
void ColorLuminancePicker::mousePressEvent( QMouseEvent *m )
{
    setVal( y2val(m->y()) );
}

void ColorLuminancePicker::setVal( int v )
{
    if ( val == v )
	return;
    val = QMAX( 0, QMIN(v,255));
    delete pix; pix=0;
    repaint( FALSE ); //###
    emit newHsv( hue, sat, val );
}

//receives from a hue,sat chooser and relays.
void ColorLuminancePicker::setCol( int h, int s )
{
    setCol( h, s, val );
    emit newHsv( h, s, val );
}

void ColorLuminancePicker::paintEvent( QPaintEvent * )
{
    int w = width() - 5;

    QRect r( 0, foff, w, height() - 2*foff );
    int wi = r.width() - 2;
    int hi = r.height() - 2;
    if ( !pix || pix->height() != hi || pix->width() != wi ) {
	delete pix;
	QImage img( wi, hi, 32 );
	int y;
	for ( y = 0; y < hi; y++ ) {
	    QColor c( hue, sat, y2val(y+coff), QColor::Hsv );
	    QRgb r = c.rgb();
	    int x;
	    for ( x = 0; x < wi; x++ )
		img.setPixel( x, y, r );
	}
	pix = new QPixmap;
	pix->convertFromImage(img);
    }
    QPainter p(this);
    p.drawPixmap( 1, coff, *pix );
    QColorGroup g = colorGroup();
    qDrawShadePanel( &p, r, g, TRUE );
    p.setPen( g.foreground() );
    p.setBrush( g.foreground() );
    QPointArray a;
    int y = val2y(val);
    a.setPoints( 3, w, y, w+5, y+5, w+5, y-5 );
    erase( w, 0, 5, height() );
    p.drawPolygon( a );
}

void ColorLuminancePicker::setCol( int h, int s , int v )
{
    val = v;
    hue = h;
    sat = s;
    delete pix; pix=0;
    repaint( FALSE );//####
}

QPoint ColorPicker::colPt()
{ return QPoint( (360-hue)*(pWidth-1)/360, (255-sat)*(pHeight-1)/255 ); }
int ColorPicker::huePt( const QPoint &pt )
{ return 360 - pt.x()*360/(pWidth-1); }
int ColorPicker::satPt( const QPoint &pt )
{ return 255 - pt.y()*255/(pHeight-1) ; }
void ColorPicker::setCol( const QPoint &pt )
{ setCol( huePt(pt), satPt(pt) ); }

ColorPicker::ColorPicker(QWidget* parent, const char* name )
    : QFrame( parent, name )
{
    hue = 0; sat = 0;
    setCol( 150, 255 );
//    setSizePolicy( QSizePolicy( QSizePolicy::Fixed, QSizePolicy::Fixed) );
	QSizePolicy* pol = new QSizePolicy();
	pol->setHorData ( QSizePolicy::Fixed );
	pol->setVerData ( QSizePolicy::Fixed );
    setSizePolicy( *pol );

    QImage img( pWidth, pHeight, 32 );
    int x,y;
    for ( y = 0; y < pHeight; y++ )
	for ( x = 0; x < pWidth; x++ ) {
	    QPoint p( x, y );
	    img.setPixel( x, y, QColor(huePt(p), satPt(p),
				       200, QColor::Hsv).rgb() );
	}
    pix = new QPixmap;
    pix->convertFromImage(img);
    setBackgroundMode( NoBackground );
}

ColorPicker::~ColorPicker()
{
    delete pix;
}

QSize ColorPicker::sizeHint() const
{
    return QSize( pWidth + 2*frameWidth(), pHeight + 2*frameWidth() );
}

void ColorPicker::setCol( int h, int s )
{
    int nhue = QMIN( QMAX(0,h), 360 );
    int nsat = QMIN( QMAX(0,s), 255);
    if ( nhue == hue && nsat == sat )
	return;
    QRect r( colPt(), QSize(20,20) );
    hue = nhue; sat = nsat;
    r = r.unite( QRect( colPt(), QSize(20,20) ) );
    r.moveBy( contentsRect().x()-9, contentsRect().y()-9 );
    //    update( r );
    repaint( r, FALSE );
}

void ColorPicker::mouseMoveEvent( QMouseEvent *m )
{
    QPoint p = m->pos() - contentsRect().topLeft();
    setCol( p );
    emit newCol( hue, sat );
}

void ColorPicker::mousePressEvent( QMouseEvent *m )
{
    QPoint p = m->pos() - contentsRect().topLeft();
    setCol( p );
    emit newCol( hue, sat );
}

void ColorPicker::drawContents(QPainter* p)
{
    QRect r = contentsRect();

    p->drawPixmap( r.topLeft(), *pix );
    QPoint pt = colPt() + r.topLeft();
    p->setPen( QPen(black) );

    p->fillRect( pt.x()-9, pt.y(), 20, 2, black );
    p->fillRect( pt.x(), pt.y()-9, 2, 20, black );

}


QValidator::State QColIntValidator::validate( QString &s, int &pos ) const
{
    State state = QIntValidator::validate(s,pos);
    if ( state == Valid ) {
	long int val = s.toLong();
	// This is not a general solution, assumes that top() > 0 and
	// bottom >= 0
	if ( val < 0 ) {
	    s = "0";
	    pos = 1;
	} else if ( val > top() ) {
	    s.setNum( top() );
	    pos = s.length();
	}
    }
    return state;
}


void ColorShowLabel::drawContents( QPainter *p )
{
    p->fillRect( contentsRect(), col );
}

void ColorShower::showAlpha( bool b )
{
    if ( b ) {
	alphaLab->show();
	alphaEd->show();
    } else {
	alphaLab->hide();
	alphaEd->hide();
    }
}



ColorShower::ColorShower( QWidget *parent, const char *name )
    :QWidget( parent, name)
{
    curCol = qRgb( -1, -1, -1 );
    QColIntValidator *val256 = new QColIntValidator( 0, 255, this );
    QColIntValidator *val360 = new QColIntValidator( 0, 360, this );

    QGridLayout *gl = new QGridLayout( this, 1, 1, 6 );
    lab = new ColorShowLabel( this );
    lab->setMinimumWidth( 60 ); //###
    gl->addMultiCellWidget(lab, 0,-1,0,0);
    connect( lab, SIGNAL( colorDropped( QRgb ) ),
	     this, SIGNAL( newCol( QRgb ) ) );
    connect( lab, SIGNAL( colorDropped( QRgb ) ),
	     this, SLOT( setRgb( QRgb ) ) );

    hEd = new QColNumLineEdit( this );
    hEd->setValidator( val360 );
    QLabel *l = new QLabel( hEd, ColorDialog::tr("Hu&e:"), this );
    l->setAlignment( AlignRight|AlignVCenter );
    gl->addWidget( l, 0, 1 );
    gl->addWidget( hEd, 0, 2 );

    sEd = new QColNumLineEdit( this );
    sEd->setValidator( val256 );
    l = new QLabel( sEd, ColorDialog::tr("&Sat:"), this );
    l->setAlignment( AlignRight|AlignVCenter );
    gl->addWidget( l, 1, 1 );
    gl->addWidget( sEd, 1, 2 );

    vEd = new QColNumLineEdit( this );
    vEd->setValidator( val256 );
    l = new QLabel( vEd, ColorDialog::tr("&Val:"), this );
    l->setAlignment( AlignRight|AlignVCenter );
    gl->addWidget( l, 2, 1 );
    gl->addWidget( vEd, 2, 2 );

    rEd = new QColNumLineEdit( this );
    rEd->setValidator( val256 );
    l = new QLabel( rEd, ColorDialog::tr("&Red:"), this );
    l->setAlignment( AlignRight|AlignVCenter );
    gl->addWidget( l, 0, 3 );
    gl->addWidget( rEd, 0, 4 );

    gEd = new QColNumLineEdit( this );
    gEd->setValidator( val256 );
    l = new QLabel( gEd, ColorDialog::tr("&Green:"), this );
    l->setAlignment( AlignRight|AlignVCenter );
    gl->addWidget( l, 1, 3 );
    gl->addWidget( gEd, 1, 4 );

    bEd = new QColNumLineEdit( this );
    bEd->setValidator( val256 );
    l = new QLabel( bEd, ColorDialog::tr("Bl&ue:"), this );
    l->setAlignment( AlignRight|AlignVCenter );
    gl->addWidget( l, 2, 3 );
    gl->addWidget( bEd, 2, 4 );

    alphaEd = new QColNumLineEdit( this );
    alphaEd->setValidator( val256 );
    alphaLab = new QLabel( alphaEd, ColorDialog::tr("A&lpha channel:"), this );
    alphaLab->setAlignment( AlignRight|AlignVCenter );
    gl->addMultiCellWidget( alphaLab, 3, 3, 1, 3 );
    gl->addWidget( alphaEd, 3, 4 );
    alphaEd->hide();
    alphaLab->hide();

    connect( hEd, SIGNAL(textChanged(const QString&)), this, SLOT(hsvEd()) );
    connect( sEd, SIGNAL(textChanged(const QString&)), this, SLOT(hsvEd()) );
    connect( vEd, SIGNAL(textChanged(const QString&)), this, SLOT(hsvEd()) );

    connect( rEd, SIGNAL(textChanged(const QString&)), this, SLOT(rgbEd()) );
    connect( gEd, SIGNAL(textChanged(const QString&)), this, SLOT(rgbEd()) );
    connect( bEd, SIGNAL(textChanged(const QString&)), this, SLOT(rgbEd()) );
}

void ColorShower::showCurrentColor()
{
    lab->setColor( currentColor() );
    lab->repaint(FALSE); //###
}

void ColorShower::rgbEd()
{
    rgbOriginal = TRUE;
    curCol = qRgb( rEd->val(), gEd->val(), bEd->val() );
    rgb2hsv(currentColor(), hue, sat, val );

    hEd->setNum( hue );
    sEd->setNum( sat );
    vEd->setNum( val );

    showCurrentColor();
    emit newCol( currentColor() );
}

void ColorShower::hsvEd()
{
    rgbOriginal = FALSE;
    hue = hEd->val();
    sat = sEd->val();
    val = vEd->val();

    curCol = QColor( hue, sat, val, QColor::Hsv ).rgb();

    rEd->setNum( qRed(currentColor()) );
    gEd->setNum( qGreen(currentColor()) );
    bEd->setNum( qBlue(currentColor()) );

    showCurrentColor();
    emit newCol( currentColor() );
}

void ColorShower::setRgb( QRgb rgb )
{
    rgbOriginal = TRUE;
    curCol = rgb;

    rgb2hsv( currentColor(), hue, sat, val );

    hEd->setNum( hue );
    sEd->setNum( sat );
    vEd->setNum( val );

    rEd->setNum( qRed(currentColor()) );
    gEd->setNum( qGreen(currentColor()) );
    bEd->setNum( qBlue(currentColor()) );

    showCurrentColor();
}

void ColorShower::setHsv( int h, int s, int v )
{
    rgbOriginal = FALSE;
    hue = h; val = v; sat = s; //Range check###
    curCol = QColor( hue, sat, val, QColor::Hsv ).rgb();

    hEd->setNum( hue );
    sEd->setNum( sat );
    vEd->setNum( val );

    rEd->setNum( qRed(currentColor()) );
    gEd->setNum( qGreen(currentColor()) );
    bEd->setNum( qBlue(currentColor()) );


    showCurrentColor();
}


//sets all widgets to display h,s,v
void ColorDialogPrivate::newHsv( int h, int s, int v )
{
    cs->setHsv( h, s, v );
    cp->setCol( h, s );
    lp->setCol( h, s, v );
}

//sets all widgets to display rgb
void ColorDialogPrivate::setCurrentColor( QRgb rgb )
{
    cs->setRgb( rgb );
    newColorTypedIn( rgb );
}

//sets all widgets exept cs to display rgb
void ColorDialogPrivate::newColorTypedIn( QRgb rgb )
{
    int h, s, v;
    rgb2hsv(rgb, h, s, v );
    cp->setCol( h, s );
    lp->setCol( h, s, v);
}


ColorDialogPrivate::ColorDialogPrivate( ColorDialog *dialog ) :
    QObject(dialog)
{
    nextCust = 0;
    const int lumSpace = 3;
    int border = 6;
    QHBoxLayout *topLay = new QHBoxLayout( dialog, border, 6 );

    initRGB();

	pWidth = 150;
	pHeight = 100;

    QVBoxLayout *rightLay = new QVBoxLayout( topLay );

    QHBoxLayout *pickLay = new QHBoxLayout( rightLay );


    QVBoxLayout *cLay = new QVBoxLayout( pickLay );
    cp = new ColorPicker( dialog );
    cp->setFrameStyle( QFrame::Panel + QFrame::Sunken );
    cLay->addSpacing( lumSpace );
    cLay->addWidget( cp );
    cLay->addSpacing( lumSpace );

    lp = new ColorLuminancePicker( dialog );
    lp->setFixedWidth( 20 ); //###
    pickLay->addWidget( lp );

    connect( cp, SIGNAL(newCol(int,int)), lp, SLOT(setCol(int,int)) );
    connect( lp, SIGNAL(newHsv(int,int,int)), this, SLOT(newHsv(int,int,int)) );

    rightLay->addStretch();

    cs = new ColorShower( dialog );
    connect( cs, SIGNAL(newCol(QRgb)), this, SLOT(newColorTypedIn(QRgb)));
    rightLay->addWidget( cs );

    QHBoxLayout *buttons;
	buttons = new QHBoxLayout( rightLay );

    QPushButton *ok, *cancel;
    ok = new QPushButton( PBDialog::tr("OK"), dialog );
    connect( ok, SIGNAL(clicked()), dialog, SLOT(accept()) );
    ok->setDefault(TRUE);
    cancel = new QPushButton( PBDialog::tr("Cancel"), dialog );
    connect( cancel, SIGNAL(clicked()), dialog, SLOT(reject()) );
    buttons->addWidget( ok );
    buttons->addWidget( cancel );
//    buttons->addStretch();
}

ColorDialog::ColorDialog(QWidget* parent, const char* name, bool modal) :
    QDialog(parent, name, modal )
{
    setSizeGripEnabled( FALSE );
    d = new ColorDialogPrivate( this );
}

QColor ColorDialog::getColor( QColor initial, QWidget *parent,
			const char *name )
{
    int allocContext = QColor::enterAllocContext();
    ColorDialog *dlg = new ColorDialog( parent, name, TRUE );  //modal
/*    if ( parent && parent->icon() && !parent->icon()->isNull() )
	dlg->setIcon( *parent->icon() );
    else if ( qApp->mainWidget() && qApp->mainWidget()->icon() && !qApp->mainWidget()->icon()->isNull() )
	dlg->setIcon( *qApp->mainWidget()->icon() );*/

    dlg->setCaption( ColorDialog::tr( "Select color" ) );
    dlg->setColor( initial );
    int resultCode = dlg->exec();
    QColor::leaveAllocContext();
    QColor result;
    if ( resultCode == QDialog::Accepted )
	result = dlg->color();
    QColor::destroyAllocContext(allocContext);
    delete dlg;
    return result;
}

QColor ColorDialog::color() const
{
    return QColor(d->currentColor());
}

void ColorDialog::setColor( QColor c )
{
    d->setCurrentColor( c.rgb() );
}

void ColorDialog::setSelectedAlpha( int a )
{
    d->showAlpha( TRUE );
    d->setCurrentAlpha( a );
}

int ColorDialog::selectedAlpha() const
{
    return d->currentAlpha();
}
