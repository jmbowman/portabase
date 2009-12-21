/*
 * mac/eventhandlers.h
 *
 * (c) 2004 by Jeremy Bowman <jmbowman@alum.mit.edu>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#include <qstring.h>

#if defined(Q_OS_MACX)
#include "../portabase.h"
#include <qmessagebox.h>
#include <Carbon/Carbon.h>

static PortaBase *pbInstance = 0;

static OSErr checkAppleEventForMissingParams(const AppleEvent& theAppleEvent)
{
    DescType returnedType;
    Size actualSize;
    OSErr err;

    switch (err = AEGetAttributePtr(&theAppleEvent, keyMissedKeywordAttr,
                                    typeWildCard, &returnedType, nil, 0,
                                    &actualSize)) {
        case errAEDescNotFound:	// If we couldn't find the error attribute
            return noErr;       // everything is ok, return noErr
        case noErr:                      // We found an error attribute, so
            return errAEEventNotHandled; // tell the client we ignored the event
        default:
            return err; // Something else happened, return it
    }
}

static pascal OSErr odocHandler(const AppleEvent* inEvent, AppleEvent*, long)
{
    AEDescList documentList;
    OSErr err = AEGetParamDesc(inEvent, keyDirectObject, typeAEList,
                               &documentList);
    if (err == noErr) {
        err = checkAppleEventForMissingParams(*inEvent);
        if (err == noErr) {
            long documentCount;
            err = AECountItems(&documentList, &documentCount);
			if (err == noErr && documentCount > 0) {
				// What kind of document is it?
                long documentIndex = 1;
				DescType returnedType;
				Size actualSize;
				err = AESizeOfNthItem(&documentList, documentIndex,
                                      &returnedType, &actualSize);
                if (err == noErr) {
                    // It's just a normal document file
                    AEKeyword keyword;
                    FSRef ref;
                    err = AEGetNthPtr(&documentList, documentIndex, typeFSRef,
                                      &keyword, &returnedType, (Ptr)&ref,
                                      sizeof(FSRef), &actualSize);
                    if (err == noErr) {
                        char buf[1024];
                        FSRefMakePath(&ref, reinterpret_cast<UInt8*>(buf), 1024);
						if (pbInstance) {
                            pbInstance->openFile(QString::fromUtf8(buf));
                        }
                    }
                }
            }
        }
        AEDisposeDesc(&documentList);
    }
    return err;
}
#endif