#import <AppKit/AppKit.h>
#include "wrapper_defines.h"

EXPORT int osal_DoFileDialogMac(char* buf,int szbuf,int is_save){
	if(is_save){
		NSSavePanel* saveDlg = [NSSavePanel savePanel];
		//[panel setAllowedFileTypes:[NSArray arrayWithObject:@"pdf"]];
		if ( [saveDlg runModal] == NSOKButton ){
			if([[[saveDlg URL] path] getCString:buf maxLength:szbuf encoding:NSUTF8StringEncoding]) {
				return 1;
			}
		}
	}else{
		NSOpenPanel* openDlg = [NSOpenPanel openPanel];
		[openDlg setCanChooseFiles:YES];
		[openDlg setCanChooseDirectories:NO];
		[openDlg setCanChooseDirectories:NO];
		//[panel setAllowedFileTypes:[NSArray arrayWithObject:@"pdf"]];
		if ( [openDlg runModal] == NSOKButton ){
			for( NSURL* URL in [openDlg URLs] ){
				if([[URL path] getCString:buf maxLength:szbuf encoding:NSUTF8StringEncoding]) {
					return 1;
				}
			}
		}
	}
	return 0;
}
