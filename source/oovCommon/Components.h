/*
 * Components.h
 *
 *  Created on: Jan 4, 2014
 *  \copyright 2014 DCBlaha.  Distributed under the GPL.
 */

#ifndef COMPONENTS_H_
#define COMPONENTS_H_

#include "NameValueFile.h"
#include "FilePath.h"

FilePaths getHeaderExtensions();
FilePaths getSourceExtensions();
FilePaths getLibExtensions();

bool isHeader(char const * const file);
bool isSource(char const * const file);
bool isLibrary(char const * const file);

/// This file is read and written by both oovcde and oovBuilder.
/// This defines each component's source and include files.
class ComponentTypesFile
    {
    public:
	enum eCompTypes
	    {
	    CT_Unknown,
	    CT_StaticLib,	// .a or .lib
	    CT_SharedLib,	// .so or .dll
	    CT_Program		// no extension or .exe
	    };

	bool read();
	bool readTypesOnly(char const * const fn);
	void setComponentNames(char const * const compNames)
	    {
	    mCompTypesFile.setNameValue("Components", compNames);
	    }
	std::vector<std::string> getComponentNames() const
	    {
	    return CompoundValueRef::parseString(
		    mCompTypesFile.getValue("Components").c_str());
	    }
	bool anyComponentsDefined() const;
	enum eCompTypes getComponentType(char const * const compName) const;
        void setComponentType(char const * const compName, char const * const typeName);
        void setComponentSources(char const * const compName,
            std::set<std::string> const &srcs);
        void setComponentIncludes(char const * const compName,
            std::set<std::string> const &incs);
	std::vector<std::string> getComponentSources(char const * const compName) const;
	std::vector<std::string> getComponentIncludes(char const * const compName) const;
	std::string getComponentBuildArgs(char const * const compName) const;
	void setComponentBuildArgs(char const * const compName, char const * const args);

	static char const * const getLongComponentTypeName(eCompTypes ct);
	static char const * const getShortComponentTypeName(eCompTypes ct)
	    { return getComponentTypeAsFileValue(ct); }
	std::string getComponentAbsolutePath(char const * const compName) const;
	void writeFile()
	    {
	    mCompTypesFile.writeFile();
	    mCompSourceListFile.writeFile();
	    }
	void writeTypesOnly(char const * const fn)
	    {
	    mCompTypesFile.setFilename(fn);
	    mCompTypesFile.writeFile();
	    }

    private:
	NameValueFile mCompTypesFile;
	NameValueFile mCompSourceListFile;

	std::vector<std::string> getComponentFiles(char const * const compName,
		char const * const tagStr) const;
        void setComponentType(char const * const compName, eCompTypes ct);
        // Setting a component below some parent must make sure the parents are unknown
	void coerceParentComponents(char const * const compName);
	// Setting a component above some child must make sure children are unknown
	void coerceChildComponents(char const * const compName);
	static std::string getCompTagName(std::string const &compName, char const * const tag);
	static char const * const getComponentTypeAsFileValue(eCompTypes ct);
	static enum eCompTypes getComponentTypeFromTypeName(
		char const * const compTypeName);
    };

/// This file is stored in the analysis directory.
/// This class does not open the file. The read must be done before calling any of
/// the other members.
class ComponentsFile:public NameValueFile
    {
    public:
	void read(char const * const fn);
	/// Get the include paths.
	std::vector<std::string> getProjectIncludeDirs() const
	    {
	    return CompoundValueRef::parseString(
		    getProjectIncludeDirsStr().c_str());
	    }
	std::vector<std::string> getAbsoluteIncludeDirs() const;
	std::vector<std::string> getExternalRootPaths() const
	    {
	    return CompoundValueRef::parseString(
		    getValue("Components-init-ext-roots").c_str());
	    }
	static void parseProjRefs(char const * const arg, std::string &rootDir,
		std::vector<std::string> &excludes);
	static bool excludesMatch(const std::string &filePath,
		const std::vector<std::string> &excludes);

    private:
	std::string getProjectIncludeDirsStr() const;
    };

#endif /* COMPONENTS_H_ */
