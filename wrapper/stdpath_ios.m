/*
<package name="stdpaths_ios">
	<target n="ios">
		<array a="ios_frameworks" n="System/Library/Frameworks/Foundation.framework"/>
	</target>
	<target n="mac">
		<array a="mac_frameworks" n="System/Library/Frameworks/Foundation.framework"/>
	</target>
</package>
*/
#import <Foundation/NSString.h>
#import <Foundation/NSPathUtilities.h>
#include <string.h>
#include "wrapper_defines.h"

static char* g_the_path=NULL;

EXPORT char* osal_getStoragePath(){
	if(!g_the_path){
		NSArray *paths = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES);
		NSString *documentsDirectory = [paths objectAtIndex:0];
		const char *cString = [documentsDirectory UTF8String];
		g_the_path=strdup(cString);
	}
	return g_the_path;
}
