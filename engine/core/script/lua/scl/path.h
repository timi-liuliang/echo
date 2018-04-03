//  Created by caolei on 2016/3/9.
#pragma once

namespace scl {
	
// get application directory (where directory the app/exe file locates)
// for ios, this is the main bundle directory
void get_application_path(char* out, const int outlen);
	
	
// get home directory
// equivalent to getenv() of C Library
void get_home_path(char* out, const int outlen);
	
	
// get document path ( based on get_home_path() )
// for ios this is the [sandbox director]/Documents
// for android TODO
// for pc, this is get_home_path()
void get_document_path(char* out, const int outlen);

	
// get_application_path() + filename
void get_application_path_filename(char* out, const int outlen, const char* const filename);


// get_document_path() + filename
void get_document_path_filename(char* out, const int outlen, const char* const filename);
	
	
} // namespace scl
