/* daICON.c */
#include <Pilot.h>
// #include "SonyChars.h"
#include "res.h"

/* define */
#define INTERVAL_TIMER		(30)
#define CREATER_ID			((DWord)'IMda')
#define CREATER_VER			100
#define LOBYTE(w)			((Byte)(w))
#define HIBYTE(w)			((Byte)(((Word)(w) >> 8) & 0xFF))
#define FRAME_PX			5
#define FRAME_PY			15
#define REAL_PX				62+11
#define REAL_PY				15+11

/* struct */
typedef struct {
	Boolean		fDown;
	Byte		cPattern[8];
	Byte		cIcon[64];
	Byte		cPen;
	Word		wResult;		// Copy or Insert
} t_Prefs;

/* prototype */
void start();
static void MainEventLoop( t_Prefs* pPrefs );
static Boolean MainHandleEvent( EventPtr event, t_Prefs* pPrefs );
static FieldPtr GetFocusObjectPtr( FormPtr activeForm );
static VoidPtr GetObjectPtr( Int objectID );
// static void MySetStrToField( int theFieldID, char* theDrawStr );
static void DrawMainFrame( t_Prefs* pPrefs );
static void DrawIconPattern( t_Prefs* pPrefs );
static void DrawIconPixel( Word x, Word y, Byte color );
static Word CharToByte( Char hi, Char lo );
static Byte CharToBit4( Char b );
static Word CheckRect( Int x, Int y );
static Boolean IsPosInRect( Int x, Int y, Int Left, Int Top, Int Right, Int Bottom );
static void IconToText( t_Prefs* pPrefs, Boolean fClipboard, FieldPtr insertField );
static Byte Bit4ToHexChar( Byte bit4 );
static void ToolFlipHorz( t_Prefs* pPrefs );
static void ToolFlipVert( t_Prefs* pPrefs );
static void ToolRotateRight( t_Prefs* pPrefs );
static void ToolRotateLeft( t_Prefs* pPrefs );
static void ToolInvert( t_Prefs* pPrefs );
static void ToolScrollUp( t_Prefs* pPrefs );
static void ToolScrollDown( t_Prefs* pPrefs );
static void ToolScrollLeft( t_Prefs* pPrefs );
static void ToolScrollRight( t_Prefs* pPrefs );

/* da main */
void start()
{
	t_Prefs tPrefs;
	FormPtr form;
	FormPtr activeForm;
	FieldPtr activeField;
	Word wStart;
	Word wEnd;
	CharPtr ptr;
	Word i, j;
	Byte hi, lo;
	Word wHex;
	Boolean fInsert;

	tPrefs.fDown = false;
	tPrefs.wResult = 0;

//	if( PrefGetAppPreferencesV10( CREATER_ID, CREATER_VER, &tPrefs, sizeof(t_Prefs) ) == false ){
		for( i = 0; i < 8; i++ ){
			tPrefs.cPattern[i] = 0x00;
		}
		for( i = 0; i < 64; i++ ){
			tPrefs.cIcon[i] = 0x00;
		}
		tPrefs.cPen = 0x00;
//	}

	wStart = 0;
	wEnd = 0;
	activeField = 0;
	activeForm = FrmGetActiveForm();
	if( activeForm != NULL ){
		activeField = GetFocusObjectPtr( activeForm );
		if( activeField != NULL ){
			FldGetSelection( activeField, &wStart, &wEnd );
		}
	}

	j = 0;
	if( activeField != 0 ){
		if( (wEnd - wStart) == 16 ){
			ptr = FldGetTextPtr( activeField );
			for( i = 0; i < 8; i++ ){
				hi = *( ptr + wStart + i * 2 + 0 );
				lo = *( ptr + wStart + i * 2 + 1 );
				wHex = CharToByte( hi, lo );
				if( (wHex & 0xFF00) == 0 ){
					tPrefs.cPattern[j] = (wHex & 0x00FF);
					j++;
				} else {
					fInsert = false;
				}
			}
		}
	}

	fInsert = false;
	if( activeField != 0 ){
		if( activeField->attr.editable != 0 ){
			if( (wEnd - wStart) == 0 ){
				fInsert = true;
			} else if( (wEnd - wStart) == 16 && j == 8 ){
				fInsert = true;
			}
		}
	}

	form = FrmInitForm( DA_FORM_ID );
	FrmSetActiveForm( form );
	FrmDrawForm( form );

	DrawMainFrame( &tPrefs );
	DrawIconPattern( &tPrefs );

	//	if( activeField != NULL ){
	if( fInsert == true ){
		CtlShowControl( (ControlPtr)GetObjectPtr( INSERT_BUTTON_ID ) );
		// CtlSetEnabled( (ControlPtr)GetObjectPtr( INSERT_BUTTON_ID ), true );
	}

	MainEventLoop( &tPrefs );

	FrmEraseForm( form );
	FrmDeleteForm( form );

//	PrefSetAppPreferencesV10( CREATER_ID, CREATER_VER, &tPrefs, sizeof(t_Prefs) );

	switch( tPrefs.wResult ){
	case COPY_BUTTON_ID:
		IconToText( &tPrefs, true, NULL );
		break;
	case INSERT_BUTTON_ID:
		InsPtEnable( true );
		FrmSetActiveForm( activeForm );
		IconToText( &tPrefs, false, activeField );
		break;
	}
}

/* event loop */
static void MainEventLoop( t_Prefs* pPrefs )
{
	EventType event;
	Word error;
	Boolean done = false;

	do {
		EvtGetEvent( &event, evtWaitForever );
		if( SysHandleEvent( &event ) == false ){
			if( MenuHandleEvent( NULL, &event, &error ) == false ){
				if( FrmHandleEvent( FrmGetActiveForm(), &event ) == false ){
					done = MainHandleEvent( &event, pPrefs );
				}
			}
		}
	} while( done == false );
}

/* event handlers */
static Boolean MainHandleEvent( EventPtr event, t_Prefs* pPrefs )
{
	Boolean done = false;
	Word x, y;
	Word wPos;
	// Char szBuf[32];

	switch( event->eType ){
	case appStopEvent:
		EvtAddEventToQueue( event );
		done = true;
		break;
	case ctlSelectEvent:
		switch( event->data.ctlSelect.controlID ){
		case COPY_BUTTON_ID:
			pPrefs->wResult = COPY_BUTTON_ID;
			// IconToClipboard( pPrefs );
			done = true;
			break;
		case INSERT_BUTTON_ID:
			pPrefs->wResult = INSERT_BUTTON_ID;
			// IconToClipboard( pPrefs );
			done = true;
			break;
		case FILP_HORZ_ID:
			ToolFlipHorz( pPrefs );
			break;
		case FILP_VERT_ID:
			ToolFlipVert( pPrefs );
			break;
		case ROTATE_RIGHT_ID:
			ToolRotateRight( pPrefs );
			break;
		case PIXEL_INVERT_ID:
			ToolInvert( pPrefs );
			break;
		case SCROLL_UP_ID:
			ToolScrollUp( pPrefs );
			break;
		case SCROLL_DOWN_ID:
			ToolScrollDown( pPrefs );
			break;
		case SCROLL_LEFT_ID:
			ToolScrollLeft( pPrefs );
			break;
		case SCROLL_RIGHT_ID:
			ToolScrollRight( pPrefs );
			break;
		}
		break;
	case penDownEvent:
		if( event->screenY < 0 ){
			done = true;
		} else {
			wPos = CheckRect( event->screenX, event->screenY );
			if( wPos != 0xFFFF ){
				pPrefs->fDown = true;
				x = HIBYTE(wPos);
				y = LOBYTE(wPos);
				if( pPrefs->cIcon[y*8+x] == 0 ){
					pPrefs->cPen = 1;
				} else {
					pPrefs->cPen = 0;
				}
				pPrefs->cIcon[y*8+x] = pPrefs->cPen;
				// DrawIconPattern( pPrefs );
				DrawIconPixel( x, y, pPrefs->cPen );
				// StrPrintF( szBuf, "%d %d", x, y );
				// MySetStrToField( SAMPLE_FILED_ID, szBuf );
			} else {
				pPrefs->fDown = false;
			}
		}
		break;
	case penMoveEvent:
		if( pPrefs->fDown == true ){
			wPos = CheckRect( event->screenX, event->screenY );
			if( wPos != 0xFFFF ){
				x = HIBYTE(wPos);
				y = LOBYTE(wPos);
				pPrefs->cIcon[y*8+x] = pPrefs->cPen;
				// DrawIconPattern( pPrefs );
				DrawIconPixel( x, y, pPrefs->cPen );
				// StrPrintF( szBuf, "%d %d", HIBYTE(wPos), LOBYTE(wPos) );
				// MySetStrToField( SAMPLE_FILED_ID, szBuf );
			} else {
				// pPrefs->fDown = false;
			}
		}
		break;
	case penUpEvent:
		if( pPrefs->fDown == true ){
			wPos = CheckRect( event->screenX, event->screenY );
			if( wPos != 0xFFFF ){
				x = HIBYTE(wPos);
				y = LOBYTE(wPos);
				pPrefs->cIcon[y*8+x] = pPrefs->cPen;
				// DrawIconPattern( pPrefs );
				DrawIconPixel( x, y, pPrefs->cPen );
				// StrPrintF( szBuf, "%d %d", HIBYTE(wPos), LOBYTE(wPos) );
				// MySetStrToField( SAMPLE_FILED_ID, szBuf );
			}
			pPrefs->fDown = false;
		}
		break;
	case keyDownEvent:
		switch( event->data.keyDown.chr ){
		// case vchrJogPageUp: // sony peg
		case 'h':
			/*
			for( y = 0; y < 8; y++ ){
				for( x = 0; x < 4; x++ ){
					wPos = pPrefs->cIcon[y*8+7-x];
					pPrefs->cIcon[y*8+7-x]	= pPrefs->cIcon[y*8+x];
					pPrefs->cIcon[y*8+x]	= wPos;
				}
			}
			DrawIconPattern( pPrefs );
			*/
			ToolFlipHorz( pPrefs );
			break;
		// case vchrJogPageDown: // spny peg
		case 'v':
			/*
			for( x = 0; x < 8; x++ ){
				for( y = 0; y < 4; y++ ){
					wPos = pPrefs->cIcon[56-y*8+x];
					pPrefs->cIcon[56-y*8+x]	= pPrefs->cIcon[y*8+x];
					pPrefs->cIcon[y*8+x]	= wPos;
				}
			}
			DrawIconPattern( pPrefs );
			*/
			ToolFlipVert( pPrefs );
			break;
		// sony peg
		/*
		case vchrJogDown:
			ToolRotateLeft( pPrefs );
			break;
		*/
		// case vchrJogUp: // sony peg
		case 't':
			/*
			// rotate left
			for( y = 0; y < 4; y++ ){
				for( x = (y*8)+y; x < ((y+1)*8)-(y*2); x++ ){
					wPos = pPrefs->cIcon[x+y*8];
					pPrefs->cIcon[x+y*8] = pPrefs->cIcon[56-x*8+y];
					pPrefs->cIcon[56-x*8+y] = pPrefs->cIcon[63-x+(7-y)*8];
					pPrefs->cIcon[63-x+(7-y)*8] = pPrefs->cIcon[x*8+7-y];
					pPrefs->cIcon[x*8+7-y] = wPos;
				}
			}
			*/
			/*
			// rotate right
			for( y = 0; y < 4; y++ ){
				for( x = y; x < ( 7 - y ); x++ ){
					wPos = pPrefs->cIcon[x+y*8];
					pPrefs->cIcon[x+y*8]		= pPrefs->cIcon[y+(7-x)*8];
					pPrefs->cIcon[y+(7-x)*8]	= pPrefs->cIcon[7-x+(7-y)*8];
					pPrefs->cIcon[7-x+(7-y)*8]	= pPrefs->cIcon[7-y+x*8];
					pPrefs->cIcon[7-y+x*8]		= wPos;
				}
			}
			DrawIconPattern( pPrefs );
			*/
			ToolRotateRight( pPrefs );
			break;
		// case vchrJogPress: // sony peg
		case 'i':
			/*
			for( x = 0; x < 64; x++ ){
				pPrefs->cIcon[x] = ( pPrefs->cIcon[x] == 0 ? 1 : 0 );
			}
			DrawIconPattern( pPrefs );
			*/
			ToolInvert( pPrefs );
			break;
		case 'u':
		case upArrowChr:		// 0x1E
		case prevFieldChr:		// 0x010C (up)
			/*
			// scroll up
			for( x = 0; x < 8; x++ ){
				y = 0;
				wPos = pPrefs->cIcon[x+y*8];
				for( y = 0; y < 7; y++ ){
					pPrefs->cIcon[x+y*8] = pPrefs->cIcon[x+(y+1)*8];
				}
				y = 7;
				pPrefs->cIcon[x+y*8] = wPos;
			}
			DrawIconPattern( pPrefs );
			*/
			ToolScrollUp( pPrefs );
			break;
		case 'd':
		case downArrowChr:		// 0x1F
		case nextFieldChr:		// 0x0103 (down)
			/*
			// scroll down
			for( x = 0; x < 8; x++ ){
				y = 7;
				wPos = pPrefs->cIcon[x+y*8];
				for( y = 7; y > 0; y-- ){
					pPrefs->cIcon[x+y*8] = pPrefs->cIcon[x+(y-1)*8];
				}
				y = 0;
				pPrefs->cIcon[x+y*8] = wPos;
			}
			DrawIconPattern( pPrefs );
			*/
			ToolScrollDown( pPrefs );
			break;
		case 'l':
		case leftArrowChr:		// 0x1C
			/*
			// scroll left
			for( y = 0; y < 8; y++ ){
				x = 0;
				wPos = pPrefs->cIcon[x+y*8];
				for( x = 0; x < 7; x++ ){
					pPrefs->cIcon[x+y*8] = pPrefs->cIcon[(x+1)+y*8];
				}
				x = 7;
				pPrefs->cIcon[x+y*8] = wPos;
			}
			DrawIconPattern( pPrefs );
			*/
			ToolScrollLeft( pPrefs );
			break;
		case 'r':
		case rightArrowChr:		// 0x1D
			/*
			// scroll right
			for( y = 0; y < 8; y++ ){
				x = 7;
				wPos = pPrefs->cIcon[x+y*8];
				for( x = 7; x > 0; x-- ){
					pPrefs->cIcon[x+y*8] = pPrefs->cIcon[(x-1)+y*8];
				}
				x = 0;
				pPrefs->cIcon[x+y*8] = wPos;
			}
			DrawIconPattern( pPrefs );
			*/
			ToolScrollRight( pPrefs );
			break;
		default:
			break;
		}
		break;
	default:
		break;
	}

	return done;
}

/* get focus object ptr */
static FieldPtr GetFocusObjectPtr( FormPtr activeForm )
{
	Word focus;
	FormObjectKind objType;

	if( FrmGetActiveFormID() == 0 ){
		return( NULL );			// I Don't know why . but it's necesarry
	}

	if( activeForm == NULL ){
		return( NULL );
	}

	focus = FrmGetFocus( activeForm );
	if( focus == noFocus ){
		return( NULL );
	}

	objType = FrmGetObjectType( activeForm, focus );
	if( objType == frmFieldObj ){
		return( FrmGetObjectPtr( activeForm, focus ));
	} else if( objType == frmTableObj ){
		return( TblGetCurrentField( FrmGetObjectPtr( activeForm, focus )));
	}

	return( NULL );
}

static VoidPtr GetObjectPtr( Int objectID )
{
	FormPtr frm = FrmGetActiveForm();
	return( FrmGetObjectPtr( frm, FrmGetObjectIndex( frm, objectID )));
}

/*
static void MySetStrToField( int theFieldID, char* theDrawStr )
{
	Handle   myTextH;
	CharPtr  myTextP;
	FieldPtr myFldPtr;

	ULong myStrSize = StrLen( theDrawStr ) + 1;
	myTextH = (Handle)MemHandleNew( (ULong)myStrSize );
	if( myTextH ){
		myTextP = (CharPtr)MemHandleLock( (VoidHand)myTextH );
		MemMove( myTextP, theDrawStr, myStrSize );
		myFldPtr = (FieldPtr)GetObjectPtr( theFieldID );
		FldFreeMemory( myFldPtr );
		FldSetTextHandle( myFldPtr, myTextH );
		FldEraseField( myFldPtr );
		FldDrawField( myFldPtr );
		MemHandleUnlock( (VoidHand)myTextH );
	}
}
*/

static void DrawMainFrame( t_Prefs* pPrefs )
{
	Word i, j;
	static const cBit[8] = { 0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01 };
	// RectangleType rect;

	// real image area
	/*
	rect.topLeft.x = REAL_PX;
	rect.topLeft.y = REAL_PY;
	rect.extent.x  = 10;
	rect.extent.y  = 10;
	WinDrawRectangleFrame( 1, &rect );
	*/

	// start pos
	i = FRAME_PX;
	j = FRAME_PY;
/*
	// horz line
	WinDrawGrayLine( i, j+1*6, i+47, j+1*6 );
	WinDrawGrayLine( i, j+2*6, i+47, j+2*6 );
	WinDrawGrayLine( i, j+3*6, i+47, j+3*6 );
	WinDrawGrayLine( i, j+4*6, i+47, j+4*6 );
	WinDrawGrayLine( i, j+5*6, i+47, j+5*6 );
	WinDrawGrayLine( i, j+6*6, i+47, j+6*6 );
	WinDrawGrayLine( i, j+7*6, i+47, j+7*6 );

	// vert line
	WinDrawGrayLine( i+1*6, j,  i+1*6, j+47 );
	WinDrawGrayLine( i+2*6, j,  i+2*6, j+47 );
	WinDrawGrayLine( i+3*6, j,  i+3*6, j+47 );
	WinDrawGrayLine( i+4*6, j,  i+4*6, j+47 );
	WinDrawGrayLine( i+5*6, j,  i+5*6, j+47 );
	WinDrawGrayLine( i+6*6, j,  i+6*6, j+47 );
	WinDrawGrayLine( i+7*6, j,  i+7*6, j+47 );

	// frame line
	WinDrawLine( i,     j,     i+48,  j     );
	WinDrawLine( i,     j+8*6, i+48,  j+8*6 );
	WinDrawLine( i,     j,     i,     j+48  );
	WinDrawLine( i+8*6, j,     i+8*6, j+48  );
*/

	// frame line
	WinDrawLine( i-1,    j-1,    i+48+1, j-1    );
	WinDrawLine( i-1,    j+48+1, i+48+1, j+48+1 );
	WinDrawLine( i-1,    j-1,    i-1,    j+48+1 );
	WinDrawLine( i+48+1, j-1,    i+48+1, j+48+1 );

	for( i = 0; i < 8; i++ ){
		for( j = 0; j < 8; j++ ){
			if( (pPrefs->cPattern[i] & cBit[j]) == 0 ){
				pPrefs->cIcon[i*8+j] = 0;
			} else {
				pPrefs->cIcon[i*8+j] = 1;
			}
		}
	}
}

static void DrawIconPattern( t_Prefs* pPrefs )
{
	Word i, j;
	// RectangleType rect;
	// static const cBit[8] = { 0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01 };

	for( i = 0; i < 8; i++ ){
		// pPrefs->cPattern[i] = 0x00;
		for( j = 0; j < 8; j++ ){
			/*
			rect.topLeft.x = (j * 6) + (FRAME_PX+1);
			rect.topLeft.y = (i * 6) + (FRAME_PY+1);
			rect.extent.x  = 5;
			rect.extent.y  = 5;
			if( pPrefs->cIcon[i*8+j] == 0 ){
				WinEraseRectangle( &rect, 0 );
				WinEraseLine( j + 61, i + 17, j + 61, i + 17 );
			} else {
				WinDrawRectangle( &rect, 0 );
				WinDrawLine( j + 61, i + 17, j + 61, i + 17 );
				// pPrefs->cPattern[i] |= cBit[j];
			}
			*/
			DrawIconPixel( j, i, pPrefs->cIcon[i*8+j] );
		}
	}
}

static void DrawIconPixel( Word x, Word y, Byte color )
{
	RectangleType rect;
	Word realx, realy;

	rect.topLeft.x = (x * 6) + (FRAME_PX+1);
	rect.topLeft.y = (y * 6) + (FRAME_PY+1);
	rect.extent.x  = 5;
	rect.extent.y  = 5;

	realx = x + REAL_PX + 1;
	realy = y + REAL_PY + 1;

	if( color == 0 ){
		WinEraseRectangle( &rect, 0 );
		WinDrawLine( rect.topLeft.x + 2, rect.topLeft.y + 2, rect.topLeft.x + 2, rect.topLeft.y + 2 );
		WinEraseLine( realx, realy, realx, realy );
	} else {
		WinDrawRectangle( &rect, 0 );
		WinDrawLine( realx, realy, realx, realy );
	}
}

static Word CharToByte( Char hi, Char lo )
{
	Byte b1, b2;
	Word result;

	result = 0;

	b1 = CharToBit4( hi );
	if( (b1 & 0xF0) == 0 ){
		b1 &= 0x0F;
	} else {
		result |= 0x8000;
	}

	b2 = CharToBit4( lo );
	if( (b2 & 0xF0) == 0 ){
		b2 &= 0x0F;
	} else {
		result |= 0x4000;
	}

	return( result + (( b1 * 0x10 ) + b2 ));
}

static Byte CharToBit4( Char b )
{
	Byte result;

	result = 0;

	if( '0' <= b && b <= '9' ){
		result = b - '0';
	} else if( 'a' <= b && b <= 'f' ){
		result = b + 10 - 'a';
	} else if( 'A' <= b && b <= 'F' ){
		result = b + 10 - 'A';
	} else {
		result = 0x80;
	}

	return( result );
}

static Word CheckRect( Int x, Int y )
{
	Word xx, yy;
	Word result;

	result = 0xFFFF;

	if( IsPosInRect( x, y, FRAME_PX+1, FRAME_PY+1, FRAME_PX+48, FRAME_PY+48 ) == true ){
		xx = (x - (FRAME_PX+1)) / 6;
		yy = (y - (FRAME_PY+1)) / 6;
		if( xx == 8 ){
			xx = 7;
		}
		if( yy == 8 ){
			yy = 7;
		}
		result = ( xx * 0x100 ) + yy;
	}

	return( result );
}

static Boolean IsPosInRect( Int x, Int y, Int Left, Int Top, Int Right, Int Bottom )
{
	if( x < Left ){
		return( false );
	}

	if( x > Right ){
		return( false );
	}

	if( y < Top ){
		return( false );
	}

	if( y > Bottom ){
		return( false );
	}

	return( true );
}

static void IconToText( t_Prefs* pPrefs, Boolean fClipboard, FieldPtr insertField )
{
	Word i, j;
	Byte szBuf[32];
	Word wStart;
	Word wEnd;
	static const cBit[8] = { 0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01 };

	for( i = 0; i < 8; i++ ){
		pPrefs->cPattern[i] = 0x00;
		for( j = 0; j < 8; j++ ){
			if( pPrefs->cIcon[i*8+j] != 0 ){
				pPrefs->cPattern[i] |= cBit[j];
			}
		}
	}

	for( i = 0; i < 8; i++ ){
		szBuf[i*2+0] = Bit4ToHexChar( pPrefs->cPattern[i] >> 4 );
		szBuf[i*2+1] = Bit4ToHexChar( pPrefs->cPattern[i] & 0xF );
	}

	if( fClipboard == true ){
		ClipboardAddItem( clipboardText, (VoidPtr)szBuf, 16 );
	}

	if( insertField != NULL ){
		FldGetSelection( insertField, &wStart, &wEnd );
		FldInsert( insertField, szBuf, 16 );
		if( (wEnd - wStart) == 16 ){
			FldSetSelection( insertField, wStart, wEnd );
		}
	}
}

static Byte Bit4ToHexChar( Byte bit4 )
{
	bit4 &= 0x0F;

	if( bit4 > 9 ){
		return( bit4 + 'A' - 10 );
	}

	return( bit4 + '0' );
}

static void ToolInvert( t_Prefs* pPrefs )
{
	Word i;
	for( i = 0; i < 64; i++ ){
		pPrefs->cIcon[i] = ( pPrefs->cIcon[i] == 0 ? 1 : 0 );
	}
	DrawIconPattern( pPrefs );
}

static void ToolFlipHorz( t_Prefs* pPrefs )
{
	Word x, y;
	Word wPos;
	for( y = 0; y < 8; y++ ){
		for( x = 0; x < 4; x++ ){
			wPos = pPrefs->cIcon[y*8+7-x];
			pPrefs->cIcon[y*8+7-x]	= pPrefs->cIcon[y*8+x];
			pPrefs->cIcon[y*8+x]	= wPos;
		}
	}
	DrawIconPattern( pPrefs );
}

static void ToolFlipVert( t_Prefs* pPrefs )
{
	Word x, y;
	Word wPos;
	for( x = 0; x < 8; x++ ){
		for( y = 0; y < 4; y++ ){
			wPos = pPrefs->cIcon[56-y*8+x];
			pPrefs->cIcon[56-y*8+x]	= pPrefs->cIcon[y*8+x];
			pPrefs->cIcon[y*8+x]	= wPos;
		}
	}
	DrawIconPattern( pPrefs );
}

static void ToolRotateRight( t_Prefs* pPrefs )
{
	Word x, y;
	Word wPos;

	// rotate right
	for( y = 0; y < 4; y++ ){
		for( x = y; x < ( 7 - y ); x++ ){
			wPos = pPrefs->cIcon[x+y*8];
			pPrefs->cIcon[x+y*8]		= pPrefs->cIcon[y+(7-x)*8];
			pPrefs->cIcon[y+(7-x)*8]	= pPrefs->cIcon[7-x+(7-y)*8];
			pPrefs->cIcon[7-x+(7-y)*8]	= pPrefs->cIcon[7-y+x*8];
			pPrefs->cIcon[7-y+x*8]		= wPos;
		}
	}

	DrawIconPattern( pPrefs );
}

static void ToolRotateLeft( t_Prefs* pPrefs )
{
	Word x, y;
	Word wPos;

	// rotate left
	for( y = 0; y < 4; y++ ){
		for( x = y; x < ( 7 - y ); x++ ){
			wPos = pPrefs->cIcon[x+y*8];
			pPrefs->cIcon[x+y*8]		= pPrefs->cIcon[7-y+x*8];
			pPrefs->cIcon[7-y+x*8]		= pPrefs->cIcon[7-x+(7-y)*8];
			pPrefs->cIcon[7-x+(7-y)*8]	= pPrefs->cIcon[y+(7-x)*8];
			pPrefs->cIcon[y+(7-x)*8]	= wPos;
		}
	}

	DrawIconPattern( pPrefs );
}

static void ToolScrollUp( t_Prefs* pPrefs )
{
	Word x, y;
	Word wPos;

	// scroll up
	for( x = 0; x < 8; x++ ){
		y = 0;
		wPos = pPrefs->cIcon[x+y*8];
		for( y = 0; y < 7; y++ ){
			pPrefs->cIcon[x+y*8] = pPrefs->cIcon[x+(y+1)*8];
		}
		y = 7;
		pPrefs->cIcon[x+y*8] = wPos;
	}
	DrawIconPattern( pPrefs );
}

static void ToolScrollDown( t_Prefs* pPrefs )
{
	Word x, y;
	Word wPos;

	// scroll down
	for( x = 0; x < 8; x++ ){
		y = 7;
		wPos = pPrefs->cIcon[x+y*8];
		for( y = 7; y > 0; y-- ){
			pPrefs->cIcon[x+y*8] = pPrefs->cIcon[x+(y-1)*8];
		}
		y = 0;
		pPrefs->cIcon[x+y*8] = wPos;
	}
	DrawIconPattern( pPrefs );
}

static void ToolScrollLeft( t_Prefs* pPrefs )
{
	Word x, y;
	Word wPos;

	// scroll left
	for( y = 0; y < 8; y++ ){
		x = 0;
		wPos = pPrefs->cIcon[x+y*8];
		for( x = 0; x < 7; x++ ){
			pPrefs->cIcon[x+y*8] = pPrefs->cIcon[(x+1)+y*8];
		}
		x = 7;
		pPrefs->cIcon[x+y*8] = wPos;
	}
	DrawIconPattern( pPrefs );
}

static void ToolScrollRight( t_Prefs* pPrefs )
{
	Word x, y;
	Word wPos;

	// scroll right
	for( y = 0; y < 8; y++ ){
		x = 7;
		wPos = pPrefs->cIcon[x+y*8];
		for( x = 7; x > 0; x-- ){
			pPrefs->cIcon[x+y*8] = pPrefs->cIcon[(x-1)+y*8];
		}
		x = 0;
		pPrefs->cIcon[x+y*8] = wPos;
	}
	DrawIconPattern( pPrefs );
}

/* eof */
