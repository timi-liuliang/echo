/*	
 *		MSZipPackage.h - MAXScript Zip Package file classes & utilities
 *
 *			Copyright (c) Autodesk, Inc, 2000.  John Wainwright.
 *
 */

#pragma once

#include "..\compiler\parser.h"

class MSZipPackage;
class DotRunParser;
class MZPExtraction;
class TabMZPExtraction;

// MZPExtraction & TabMZPExtraction, for maintaing a table of
//   file extractions so far giving name in .zip and local extracted name
class MZPExtraction
{
public:
	MSTR zip_name;
	MSTR extracted_name;

	MZPExtraction(const MCHAR* zip_name, const MCHAR* extracted_name) :
		zip_name(zip_name), extracted_name(extracted_name) { }
};

class TabMZPExtraction : public Tab<MZPExtraction*>
{
public:
	~TabMZPExtraction();

	MCHAR* find(const MCHAR* zip_name);
	void   add(const MCHAR* zip_name, const MCHAR* extracted_name);
};

// MSZipPackage: instances represent open .mzp package files
class MSZipPackage : public Value
{
public:
	enum clear_modes { CLEAR_NOW, CLEAR_ON_MAX_EXIT, CLEAR_ON_RESET, KEEP, };   // clear temp modes

	MSTR	file_name;			// zip package file name
	MSTR	package_name;		// parsed from mzp.run file...
	MSTR	description;
	int		version;
	MSTR	extract_dir;		// current extract dir
	MSTR	drop_file;			// primary drop file (if any in found)
	TabMZPExtraction extractions; // currently extracted files
	clear_modes clear_mode;
	WORD	flags;				// flag bits

	MSZipPackage(const MCHAR* file_name) : file_name(file_name), flags(0), clear_mode(CLEAR_ON_MAX_EXIT) { tag = INTERNAL_CLASS_TAG; } 

	void	collect() { delete this; }

	// main zip package file in function
	ScripterExport static bool file_in_package(const MCHAR* file_name, MSTR* extract_dir=NULL);
	static int WINAPI output_callback(char far *buf, unsigned long size);

	// package SearchFile
	ScripterExport static BOOL search_current_package(const MCHAR* file_name, MCHAR* found_path);

	// extraction
	bool	extract_to(const MCHAR* dir);
	MCHAR*	find_extraction(const MCHAR* zip_name) { return extractions.find(zip_name); }
	void	add_extraction(const MCHAR* zip_name, const MCHAR* extracted_name);

	// directory & file manipulation
	ScripterExport static MSTR	expand_dir(const MCHAR* dir);
	ScripterExport static MSTR	expand_file(const MCHAR* file_name);
	MSTR	expand_dir_for_extraction(const MCHAR* dir);
	MSTR	expand_file_for_extraction(const MCHAR* file_name);

	// running
	void	run_all_scripts();
	bool	run_script(const MCHAR* zip_name);
};

// MZP flags
#define MZP_DONT_RUN		0x0001		// just loading, don't run any scripts
#define MZP_HAVE_DROPFILE	0x0002		// 'drop' command encountered
#define MZP_HAVE_OPEN		0x0004		// 'open' command encountered

// interpret copy() function type bits...
#define MZP_COPY		0x00
#define MZP_MOVE		0x01
#define MZP_FILES		0x00
#define MZP_TREE		0x02
#define MZP_NOREPLACE	0x04

// parser specialization to parse & run mzp.run control file
class DotRunParser : public Parser
{
	MSZipPackage* mzp;

public:

	DotRunParser (MSZipPackage* mzp) : mzp(mzp) { }

	// parse & run mzp.run file
	bool	interpret(const MCHAR* src);

	// parser production functions
	void	name();
	void	version();
	void	description();
	void	extract();
	void	run();
	void	drop();
	void	open();
	void	merge();
	void	xref();
	void	import();
	void	copy(BYTE type);
	void	clear();
	void	keep();

	// utils
	void	copy_files(const MCHAR* from, const MCHAR* to, BYTE type);
	ScripterExport static bool	create_dir(const MCHAR* dir);
};

