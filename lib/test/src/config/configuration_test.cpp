/*
 configuration_test.cpp     MindForger configuration test

 Copyright (C) 2016-2021 Martin Dvorak <martin.dvorak@mindforger.com>

 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License
 as published by the Free Software Foundation; either version 2
 of the License, or (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#include "../../../src/representations/markdown/markdown_configuration_representation.h"

#include <gtest/gtest.h>
#include "../test_gear.h"

extern char* getMindforgerGitHomePath();

using namespace std;

TEST(ConfigurationTestCase, FromConstructor)
{
    string repositoryPath{"/lib/test/resources/basic-repository"};
    repositoryPath.insert(0, getMindforgerGitHomePath());

    m8r::Configuration& config = m8r::Configuration::getInstance();
    config.clear();
    config.setConfigFilePath("/tmp/cfg-ctc-fc.md");
    config.setActiveRepository(config.addRepository(m8r::RepositoryIndexer::getRepositoryForPath(repositoryPath)));

    cout << endl << "Active repository:" << endl << "  " << config.getActiveRepository()->getDir();
    cout << endl << "Repositories[" << config.getRepositories().size() << "]:";
    for(auto& r:config.getRepositories()) {
        cout << endl << "  " << r.first;
    }
    cout << endl;

    EXPECT_GE(config.getRepositories().size(), 1);
}

TEST(ConfigurationTestCase, FromEnvironment)
{
    // set MINDFORGER_REPOSITORY environment variable
    const char* envVarName = "MINDFORGER_REPOSITORY";
    const char* relativeRepositoryPath = "/lib/test/resources/basic-repository";
    char* envVar = new char[strlen(envVarName)+1+strlen(getMindforgerGitHomePath())+strlen(relativeRepositoryPath)+1];
    envVar[0] = 0;
    strcat(envVar, envVarName);
    strcat(envVar, "=");
    strcat(envVar, getMindforgerGitHomePath());
    strcat(envVar, relativeRepositoryPath);
    putenv(envVar);
    cout << "Setting env:" << endl << "  " << envVar << endl;

    m8r::Configuration& config = m8r::Configuration::getInstance();
    config.clear();
    config.setConfigFilePath("/tmp/cfg-ctc-fe.md");

    config.setActiveRepository(
        config.addRepository(
            m8r::RepositoryIndexer::getRepositoryForPath(config.getRepositoryPathFromEnv())));

    if(config.getActiveRepository()) {
        cout << endl << "Active repository:" << endl << "  " << config.getActiveRepository()->getDir();
        cout << endl << "Repositories[" << config.getRepositories().size() << "]:";
        for(auto& r:config.getRepositories()) {
            cout << endl << "  " << r.first;
        }
        EXPECT_GE(config.getRepositories().size(), 1);
    } else {
        FAIL() << "Repository environment variable is NOT set!" << endl;
    }
    cout << endl;

    // unset environment variable
#ifdef _WIN32
    envVar[0] = 0;
    strcat(envVar, m8r::ENV_VAR_M8R_REPOSITORY);
    strcat(envVar, "=");
   _putenv(envVar);
#else
    unsetenv(m8r::ENV_VAR_M8R_REPOSITORY);
#endif //_WIN32
    delete[] envVar;
}

TEST(ConfigurationTestCase, SaveDefaultConfig)
{
    m8r::File file{"/tmp/.mindforger.md.default"};
    m8r::MarkdownConfigurationRepresentation configRepresentation{};

    configRepresentation.save(file);

    // asserts
    string* asString = m8r::fileToString(file.getName());
    EXPECT_NE(std::string::npos, asString->find("Theme: dark"));
    EXPECT_NE(std::string::npos, asString->find("Time scope: 0y0m0d0h0m"));
    EXPECT_NE(std::string::npos, asString->find("Editor syntax highlighting: yes"));
    EXPECT_NE(std::string::npos, asString->find("Save reads metadata: yes"));
    EXPECT_NE(std::string::npos, asString->find("Active repository: ~/mindforger-repository"));
    EXPECT_NE(std::string::npos, asString->find("Repository: ~/mindforger-repository"));
    delete asString;
}

TEST(ConfigurationTestCase, SaveAndLoad)
{
    // GIVEN
    m8r::TestSandbox box{""};
    string mdFilename{"custom-repository-single-file.md"};
    string mdFilePath{box.addMdFile(mdFilename)};
    cout <<
    "Test directories:" << endl <<
    "  home   :" << box.testHomePath << endl <<
    "  MF cfg :" << box.configPath << endl <<
    "  MD file:" << mdFilePath << endl
    ;

    string timeScopeAsString{};
    m8r::MarkdownConfigurationRepresentation configRepresentation{};
    m8r::Configuration& c = m8r::Configuration::getInstance();

    string backupFile = c.getConfigFilePath();
    string backupTheme = c.getUiThemeName();
    m8r::TimeScope backupTimeScope = c.getTimeScope();
    bool backupReadsMetadata = c.isSaveReadsMetadata();
    bool backupNotebookButton = c.isUiEditorEnableSyntaxHighlighting();
    m8r::Repository* backupActiveRepository;
    if(c.isActiveRepository()) {
        backupActiveRepository = new m8r::Repository(*c.getActiveRepository());
    } else {
        backupActiveRepository = nullptr;
    }

    /*
     * SAVE: mode ~ single MD file edit
     */

    c.setConfigFilePath(box.configPath);
    c.setUiThemeName("CRAZYCOLORS");
    m8r::TimeScope ts{1,2,33,4,55};
    c.setTimeScope(ts);
    c.setSaveReadsMetadata(false);
    c.setUiEditorEnableSyntaxHighlighting(false);
    m8r::Repository* r = new m8r::Repository{
        box.testHomePath,
        m8r::Repository::RepositoryType::MARKDOWN,
        m8r::Repository::RepositoryMode::FILE,
        mdFilename
    };
    c.setActiveRepository(c.addRepository(r));

    m8r::RepositoryConfiguration& rc{c.initRepositoryConfiguration()};
    string oName{"Repository configuration test organizer"};
    m8r::Organizer* o = new m8r::Organizer{oName};
    // organizer will be deleted by destructor
    rc.addOrganizer(o);

    configRepresentation.save(c);

    // asserts
    std::unique_ptr<string> asString{m8r::fileToString(c.getConfigFilePath())};
    EXPECT_NE(std::string::npos, asString.get()->find("Theme: CRAZYCOLORS"));
    EXPECT_NE(std::string::npos, asString.get()->find("Time scope: 1y2m33d4h55m"));
    EXPECT_NE(std::string::npos, asString.get()->find("Editor syntax highlighting: no"));
    EXPECT_NE(std::string::npos, asString.get()->find("Save reads metadata: no"));
    EXPECT_NE(std::string::npos, asString.get()->find(string("Active repository: ") + mdFilePath));
    EXPECT_NE(std::string::npos, asString.get()->find(string("Repository: ") + mdFilePath));
    // r deleted by configuration destructor
    EXPECT_FALSE(c.hasRepositoryConfiguration());
    EXPECT_EQ("", c.getRepositoryConfigFilePath());
    EXPECT_EQ(0, c.getRepositoryConfiguration().getOrganizers().size());

    /*
     * LOAD
     */

    c.setConfigFilePath(box.configPath);

    bool loaded = configRepresentation.load(c);

    // asserts
    ASSERT_TRUE(loaded);
    EXPECT_EQ("CRAZYCOLORS", c.getUiThemeName());
    c.getTimeScope().toString(timeScopeAsString);
    EXPECT_EQ("1y2m33d4h55m", timeScopeAsString);
    EXPECT_FALSE(c.isSaveReadsMetadata());
    EXPECT_FALSE(c.isUiEditorEnableSyntaxHighlighting());

    EXPECT_GE(c.getRepositories().size(), 1);
    EXPECT_NE(c.getRepositories().end(), c.getRepositories().find(mdFilePath));
    c.getRepositories().find(mdFilePath)->second->print();
    EXPECT_EQ(mdFilePath, c.getRepositories().find(mdFilePath)->second->getPath());
    EXPECT_EQ(box.repositoryPath, c.getRepositories().find(mdFilePath)->second->getDir());
    EXPECT_EQ(mdFilename, c.getRepositories().find(mdFilePath)->second->getFile());
    EXPECT_EQ(m8r::Repository::RepositoryMode::FILE, c.getRepositories().find(mdFilePath)->second->getMode());

    EXPECT_NE(nullptr, c.getActiveRepository());
    c.getActiveRepository()->print();
    EXPECT_EQ(mdFilePath, c.getActiveRepository()->getPath());
    EXPECT_EQ(box.repositoryPath, c.getActiveRepository()->getDir());
    EXPECT_EQ(mdFilename, c.getActiveRepository()->getFile());
    EXPECT_EQ(m8r::Repository::RepositoryMode::FILE, c.getActiveRepository()->getMode());
    // r deleted by configuration destructor

    // configuration cleanup
    c.setConfigFilePath(backupFile);
    c.setUiThemeName(backupTheme);
    c.setTimeScope(backupTimeScope);
    c.setSaveReadsMetadata(backupReadsMetadata);
    c.setUiEditorEnableSyntaxHighlighting(backupNotebookButton);
    if(backupActiveRepository) {
        c.setActiveRepository(c.addRepository(backupActiveRepository));
    } else {
        c.setActiveRepository(nullptr);
    }
}
