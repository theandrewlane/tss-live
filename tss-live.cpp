#include <cstdio>
#include <iostream>
#include <fstream>
#include "include/twitcurl.h"
#include <unistd.h>
#include "./include/json.hpp"


using json = nlohmann::json;

twitCurl twit;

void usage() {
    printf("\nUsage: tss-live (-r searchResultLimit | -t)\n");
}


int error() {
    std::string errResponse;
    printf("\nOpps, there was an error...");
    twit.getLastWebResponse(errResponse);
    printf("\nAre you connected to the internet?\nError: %s", errResponse.c_str());
    return 0;
}

void auth() {
    printf("\nAuthorizing user...");
    char buffer[1024];
    std::ifstream getCredentials, getKeys;
    std::string username, password, cKey, cSec, oKey, oSec;

    //Read from a file to get the credentials (just to be safe)
    getCredentials.open(".tss-live-credentials");
    if (getCredentials.is_open()) {
        memset(buffer, 0, 1024);
        getCredentials >> buffer;
        username = buffer;
        memset(buffer, 0, 1024);
        getCredentials >> buffer;
        password = buffer;

    }

    //Close Stream
    getCredentials.close();
    //Did user specify credentials?
    if (username.size() && password.size()) {
        //Set The twitter credentials
        twit.setTwitterUsername(username);
        twit.setTwitterPassword(password);

        //Read Keys from file just to be safe
        getKeys.open(".tss-live-keys");
        if (getKeys.is_open()) {
            memset(buffer, 0, 1024);
            getKeys >> buffer;
            cKey = buffer;
            memset(buffer, 0, 1024);
            getKeys >> buffer;
            cSec = buffer;
            memset(buffer, 0, 1024);
            getKeys >> buffer;
            oKey = buffer;
            memset(buffer, 0, 1024);
            getKeys >> buffer;
            oSec = buffer;
        }
        if (cKey.size() && cSec.size() && oKey.size() && oSec.size()) {

            //Set the consumer token and secret from the app
            twit.getOAuth().setConsumerKey(cKey);
            twit.getOAuth().setConsumerSecret(cSec);

            //Set our auth tokens
            twit.getOAuth().setOAuthTokenKey(oKey);
            twit.getOAuth().setOAuthTokenSecret(oSec);

            //use the auth token to get a request token
            twit.oAuthAccessToken();
            printf("\nUser authorized...");
        } else {
            perror("\nNo Twitter keys found, please provide keys and try again");
            error();
        }
    } else {
        perror("\nNo Twitter credentials found, please add credentials and try again");
        error();
    }

}

//Logs the Usage of this program
void tssLiveUsageLog() {
    std::string tweet;
    char hostname[1024];
    time_t t = std::time(0);

    gethostname(hostname, 1024);

    long int now = static_cast<long int> (t);
    //Add timestamp to tweet to make it unique, otherwise twitter won't allow it
    tweet = std::to_string(now);
    tweet = tweet + ":";
    tweet = tweet + strcat(hostname, " is using tss-live!");
    //Send a tweet - consisting of the users hostname
    twit.statusUpdate(tweet);
}

//Runs a search given a query and result size
void twitSearch(std::string query, std::string resultSize) {
    json j;
    std::string twitSearchResponse;
    std::ofstream searchResStream;
    printf("\nExecuting a twitter search...");

    //Open the results file for writing
    searchResStream.open("tssLive_results.json");

    //Run the search
    twit.search(query, resultSize);

    //Get the response and parse
    twit.getLastWebResponse(twitSearchResponse);
    auto j3 = json::parse(twitSearchResponse);

    //Write results to file, and format it while we're at it
    for (json::iterator it = j3.begin(); it != j3.end(); ++it) {
        ++it; //Skip the first JSON object - it's not part of the results
        searchResStream << std::setw(4) << it.value() << "\n";
    }

    //close the stream
    searchResStream.close();
    printf("\nSearch successful! Results written to file: ./tssLive_searchResults.json");
}

//Get the current trends
void twitTrends() {
    std::ofstream trendResStream;
    std::string twitTrendsResponse;
    printf("\nGetting the latest trends");

    //Open the results file for writing
    trendResStream.open("tssLive_trends.json");

    //Run the search
    twit.trendsCurrentGet();
    //Get the response and parse
    twit.getLastWebResponse(twitTrendsResponse);
    auto j3 = json::parse(twitTrendsResponse);

    //Write results to file, and format it while we're at it
    for (json::iterator it = j3.begin(); it != j3.end(); ++it) {
        trendResStream << std::setw(4) << it.value() << "\n";
    }

    //close the stream
    trendResStream.close();
    printf("\nSuccessfully retrieved trends! Results written to file: ./tssLive_trendResults.json");
}

int main(int argc, char *argv[]) {
    json j;
    std::string resultSize;
    std::string response, tweet, query;

    bool isSearching = false;
    bool isTrending = false;

    //Check user input
    if (argc > 1) {
        for (int i = 1; i < argc; i += 1) {
            if (0 == strncmp(argv[i], "-r", strlen("-r"))) {
                if (argc != 3) {
                    usage();
                    return 0;
                }
                resultSize = argv[i + 1];
                isSearching = true;
            } else if (0 == strncmp(argv[i], "-t", strlen("-t"))) {
                resultSize = argv[1];
                isTrending = true;
            }
        }
        if (0 == resultSize.length() && isSearching) {
            usage();
            return 0;
        }

    } else {
        usage();
        return 0;
    }

    printf("Welcome to tssLive!");

    //Auth app with twitter
    auth();

    //Run a search, if that's what the user wants
    if (isSearching) {
        //Searching a string
        printf("\nEnter string to search: ");
        std::getline(std::cin, query);
        twitSearch(query, resultSize);
    }

    //Run a trend, if that's what the user wants
    if (isTrending) {
        twitTrends();
    }

    //Log every time this app is run - via a tweet of course :)
    tssLiveUsageLog();
    //Make it look clean
    printf("\n");

    //Great success!
    return 1;
}
