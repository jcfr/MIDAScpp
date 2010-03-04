/*=========================================================================

  Copyright (c) Kitware Inc. All rights reserved.
  See Copyright.txt or http://www.Kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#include "mwsRestAPI.h"

#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include "sys/stat.h"
#include "mwsRestXMLParser.h"

namespace mws {

/** Constructor */
RestAPI::RestAPI()
{
  m_Initialized = false; 
  m_OutputMode = BUFFER;
  m_CurlCode = CURLE_OK;
  output_filestream = NULL;
  input_stream = NULL;
  m_cURL = NULL;
  upload_unique_id = "";
  fprogress = NULL;
  m_Verbose = false;
  m_XMLParser = NULL;
}

/** Desctructor */
RestAPI::~RestAPI()
{
  this->Finalize();
}

/** Set cURL in verbose mode */
void RestAPI::SetVerbose(bool verbose)
{
  m_Verbose = verbose;
}

/** Initialize cURL */
bool RestAPI::Initialize()
{
  if (!m_Initialized)
    {
    //curl_global_init(CURL_GLOBAL_ALL);
    m_Initialized = true; 
    }
  if(!m_cURL)
    {
    m_cURL = curl_easy_init(); 
    }
  if(!m_cURL)
    {
    return false;
    }
  return true;
}

/** Set the XML parser */
void RestAPI::SetXMLParser(mws::RestXMLParser* parser)
{
  m_XMLParser = parser;
} 
 
/** Set the cURL options */
bool RestAPI::SetCurlOptions(const char* url, 
                             const char* authentication)
{
  if(!m_cURL)
    {
    std::cout << "SetStdCurlOption m_cURL is not initialized" << std::endl;
    return false;
    }
  curl_easy_setopt(m_cURL, CURLOPT_WRITEFUNCTION, Curl_output_function);
  curl_easy_setopt(m_cURL, CURLOPT_WRITEDATA, this);
  curl_easy_setopt(m_cURL, CURLOPT_SSL_VERIFYHOST, 1);
  curl_easy_setopt(m_cURL, CURLOPT_SSL_VERIFYPEER, false);
  curl_easy_setopt(m_cURL, CURLOPT_URL, url);

  if(m_Verbose)
    {
    curl_easy_setopt(m_cURL, CURLOPT_VERBOSE, 1); 
    }
    
  curl_easy_setopt(m_cURL, CURLOPT_NOPROGRESS, this->fprogress == NULL);
  curl_easy_setopt(m_cURL, CURLOPT_PROGRESSFUNCTION, this->fprogress);
  curl_easy_setopt(m_cURL, CURLOPT_PROGRESSDATA, this->fprogress_data);

  curl_easy_setopt(m_cURL, CURLOPT_FOLLOWLOCATION, true);

  if (authentication != "")
    {
    curl_easy_setopt(m_cURL, CURLOPT_HTTPAUTH, CURLAUTH_BASIC);
    curl_easy_setopt(m_cURL, CURLOPT_USERPWD,authentication);
    }
  return true;  
}

/** Return the status of the current API */
void RestAPI::PrintStatus()
{
  std::cout << "\tm_Initialized:"  << this->m_Initialized << std::endl;
  std::cout << "\toutput_buffer:"     << (this->output_buffer.empty() ? "NOT" : "") << "NULL: " << this->output_buffer /*kwutils::to_string<std::string*>(this->output_buffer)*/  << std::endl;
  std::cout << "\tm_CurlCode:"          << this->m_CurlCode << std::endl;
  std::cout << "\tIO_MODE::FILE:"     << FILE << std::endl;
  std::cout << "\tIO_MODE::BUFFER:"   << BUFFER << std::endl;
  std::cout << "\tm_OutputMode:"       << this->m_OutputMode << std::endl;
  std::cout << "\tinput_mode:"        << this->input_mode << std::endl;
}

//--------------------------------------------------------------------------------------------------
bool RestAPI::PerformCurl()
{
  if(m_cURL == NULL)
    {
    return false;
    }

  if(m_OutputMode == BUFFER)
    {
    this->ResetOutputBuffer();
    }     
  
  m_CurlCode = curl_easy_perform(m_cURL);
  if ( m_CurlCode != CURLE_OK )
    {
    std::cout << "Cannot run cURL: "  << curl_easy_strerror(m_CurlCode) << std::endl;
    return false;
    }
  return true;
}

//--------------------------------------------------------------------------------------------------
bool RestAPI::Execute(const char*  url, 
                      const char*  post_data,
                      const char * authentication)
{
  if(!m_cURL)
    {
    std::cout << "Execute: cURL not initialized" << std::endl;
    return false;
    }  
 
  std::string URL = url;
  if(!m_ServerUrl.empty())
    {
    URL = m_ServerUrl + URL; 
    }
  
  // setup
  curl_easy_reset(m_cURL);
  m_OutputMode = BUFFER;

  this->SetCurlOptions(URL.c_str(), authentication);  

  if(post_data != NULL)
    {
    curl_easy_setopt(m_cURL, CURLOPT_POST, 1);
    curl_easy_setopt(m_cURL, CURLOPT_POSTFIELDS,post_data);
    }
  else 
    {
    curl_easy_setopt(m_cURL, CURLOPT_HTTPGET, 1);
    }

  m_XMLParser->Initialize();
  if(!this->PerformCurl())
    {
    return false;
    }
  m_XMLParser->Finalize();
   
  return true;  
}

//--------------------------------------------------------------------------------------------------
bool RestAPI::Download(const std::string &filename, std::string url, IO_MODE outputMode,
                       curl_progress_callback fprogress, void * fprogress_data,
                       const char* authentication)
{
  std::string URL = url;
  if(!m_ServerUrl.empty())
    {
    URL = m_ServerUrl + URL; 
    }

  m_OutputMode = outputMode;
  // prepare output
  switch(m_OutputMode)
    {
    case BUFFER :
      ResetOutputBuffer();
      break;
    case FILE :
      this->output_filestream = new std::ofstream;
      this->output_filestream->open(filename.c_str(), std::ios::binary);
      if(!this->output_filestream->is_open())
        {
        std::cerr << "Cannot open local file for writing: " << filename.c_str() << std::endl;
        return false;
        }
      break;
    }

  curl_easy_reset(m_cURL);

  this->SetCurlOptions(URL.c_str(), authentication);
  curl_easy_setopt(m_cURL, CURLOPT_HTTPGET, 1);
  curl_easy_setopt(m_cURL, CURLOPT_PROGRESSDATA, fprogress_data == NULL ?
                                                 this->fprogress_data : fprogress_data);
  curl_easy_setopt(m_cURL, CURLOPT_PROGRESSFUNCTION, fprogress == NULL ?
                                                 this->fprogress : fprogress);

  bool success = this->PerformCurl();

  switch(m_OutputMode)
    {
    case BUFFER :
      break;
    case FILE :
      this->output_filestream->close();
      this->output_filestream = NULL;
      break;
    }
  return success;  
 }


//-----------------------------------------------------------------------------
// return size of file; also returns zero if no file exists
unsigned long RestAPI::GetFileLength(const char* filename)
{
  struct stat fs;
  if (stat(filename, &fs) != 0)
    {
    return 0;
    }
  else
    {
    return static_cast<unsigned long>(fs.st_size);
    }
}

//--------------------------------------------------------------------------------------------------
bool RestAPI::UploadPost(const char* filename, std::string url, curl_progress_callback fprogress,
                         void * fprogress_data, const char* authentication)
{
  std::string URL = url;
  if(!m_ServerUrl.empty())
    {
    URL = m_ServerUrl + URL;
    }
    
  curl_easy_reset(m_cURL);

  struct curl_httppost *formpost=NULL;
  struct curl_httppost *lastptr=NULL;
  struct curl_slist *headerlist=NULL;
  static const char buf[] = "Expect:";

  m_OutputMode = BUFFER;
  this->SetCurlOptions(URL.c_str(), authentication);
  curl_easy_setopt(m_cURL, CURLOPT_POST, 1);
  curl_formadd(&formpost,
               &lastptr,
               CURLFORM_COPYNAME, "file",
               CURLFORM_FILE, filename,
               CURLFORM_END);
  curl_easy_setopt(m_cURL, CURLOPT_HTTPPOST, formpost);
 
  /* only disable 100-continue header if explicitly requested */ 
  headerlist = curl_slist_append(headerlist, buf);
  curl_easy_setopt(m_cURL, CURLOPT_HTTPHEADER, headerlist);
  
  /* initalize custom header list (stating that Expect: 100-continue is not
     wanted */ 
  headerlist = curl_slist_append(headerlist, buf);

  m_XMLParser->Initialize();
  bool success = this->PerformCurl();
  m_XMLParser->Finalize();

  success &= (m_XMLParser->GetErrorCode() == 0);
  
  curl_formfree(formpost);
  
  /* free slist */ 
  curl_slist_free_all (headerlist);
  
  return success;
}

//--------------------------------------------------------------------------------------------------
bool RestAPI::Upload(const std::string &data, std::string url, curl_progress_callback fprogress,
                     void * fprogress_data, const char* authentication)
{
  std::string URL = url;
  if(!m_ServerUrl.empty())
    {
    URL = m_ServerUrl + URL; 
    }
  
  unsigned long datasize = 0; 
  std::ifstream * input_filestream = NULL;

  if (this->upload_unique_id.empty())
    {
    //TODO: FIX throw RestAPIException(__FILE__, __LINE__, __func__,"Upload unique identifier is NOT set");
    }

  // prepare input
  switch(input_mode)
    {
    case BUFFER:
      //Logger::debug(STR("\tdata(STRING:")+kwutils::to_string<size_t>(data.size())+")");
      datasize = data.size(); 
      this->input_stream = new std::stringstream(data, std::stringstream::out);
      break; 
    case FILE:
      //Logger::debug("\tdata(filename:"+data+")");
      datasize = this->GetFileLength(data.c_str());
      input_filestream = new std::ifstream;
      input_filestream->open(data.c_str(), std::ios::binary);
      if(!input_filestream->is_open())
        {
        //TODO: FIX throw RestAPIException(__FILE__, __LINE__, __func__, "Failed to open file:" + data);
        } 
      this->input_stream = input_filestream; 
      break;
    }

  curl_easy_reset(m_cURL);

  m_OutputMode = BUFFER;
  this->SetCurlOptions(URL.c_str(), authentication);

  void * read_function_data = (void*) this; 
  curl_easy_setopt(m_cURL, CURLOPT_UPLOAD, true);
  curl_easy_setopt(m_cURL, CURLOPT_INFILESIZE, datasize);
  curl_easy_setopt(m_cURL, CURLOPT_READFUNCTION, Curl_read_function);
  curl_easy_setopt(m_cURL, CURLOPT_READDATA, read_function_data);

  // remove 'Expect: 100-continue' header
  struct curl_slist *headers = NULL;
  headers = curl_slist_append(headers, "Expect: ");
  curl_easy_setopt(m_cURL, CURLOPT_HTTPHEADER, headers);

  //TODO: FIX  M_ASSERT(this->fprogress_data != NULL);
  //TODO: FIX  M_ASSERT(this->fprogress != NULL);

  //Logger::debug("\tfprogress:" + kwutils::to_string<curl_progress_callback>(this->fprogress)); 
  //Logger::debug("\tfprogress_data:" + kwutils::to_string<void*>(this->fprogress_data)); 

  curl_easy_setopt(m_cURL, CURLOPT_PROGRESSDATA, fprogress_data == NULL ?
                                                this->fprogress_data : fprogress_data);
  curl_easy_setopt(m_cURL, CURLOPT_PROGRESSFUNCTION, fprogress == NULL ?
                                                    this->fprogress : fprogress);

  bool success = this->PerformCurl();
  
  if (input_filestream != NULL)
    {
    input_filestream->close();
    input_filestream = NULL; 
    }
  curl_slist_free_all(headers);

  return success;
}

/** Finalize */
void RestAPI::Finalize()
{
  if(!m_Initialized)
    {
    return;
    }
    
  if (m_cURL != NULL)
    {
    curl_easy_cleanup(m_cURL); 
    }
    
  if (this->input_stream != NULL )
    {
    this->input_stream->clear();
    delete this->input_stream; 
    this->input_stream = NULL; 
    }
  m_cURL = NULL;
  m_Initialized = false;  
}


//--------------------------------------------------------------------------------------------------
RestAPI::IO_MODE RestAPI::GetOutputMode()
{
  return m_OutputMode; 
}

//--------------------------------------------------------------------------------------------------
RestAPI::IO_MODE RestAPI::GetInputMode()
{
  return this->input_mode; 
}

/** Set the base URL */
void RestAPI::SetServerUrl(const char*  baseurl)
{
  std::string temp(baseurl);
  // start with 'http://' ou 'https://' :
  if(temp.substr(0, 7).compare("http://")!=0 && temp.substr(0, 8).compare("https://")!=0 )
    {
    temp.insert(0, "http://");
    }
  // end with a '/' :
  if(temp.substr(temp.length()-1).compare("/")!=0)
    {
    temp.append("/");
    }
  m_ServerUrl = temp;
}

/** Get the base URL */
const char* RestAPI::GetServerUrl()
{
  return m_ServerUrl.c_str();
}
//--------------------------------------------------------------------------------------------------
void RestAPI::SetInputMode(IO_MODE input_mode)
  {
  this->input_mode = input_mode; 
  }


//--------------------------------------------------------------------------------------------------
std::ofstream* RestAPI::GetOutputFileStream()
  {
  return this->output_filestream; 
  }

//--------------------------------------------------------------------------------------------------
std::istream* RestAPI::GetInputStream()
  {
  return this->input_stream; 
  }

//--------------------------------------------------------------------------------------------------
std::string &RestAPI::GetOutputBuffer()
  {
  return this->output_buffer; 
  }

//--------------------------------------------------------------------------------------------------
void RestAPI::ResetOutputBuffer()
  {
  output_buffer.clear();
  }

//--------------------------------------------------------------------------------------------------
const std::string& RestAPI::GetUploadUniqueId()
  {
  
  //Logger::debug(this->upload_unique_id); 
  
  return this->upload_unique_id; 
  }

//--------------------------------------------------------------------------------------------------
void RestAPI::SetUploadUniqueId(const std::string &value)
  {
  this->upload_unique_id.assign(value); 
  }

//--------------------------------------------------------------------------------------------------
void RestAPI::SetProgressCallback(curl_progress_callback fprogress, void * fprogress_data)
  {
  
  this->fprogress = fprogress; 
  this->fprogress_data = fprogress_data; 
  
  }

bool RestAPI::Parse(const char* buffer,unsigned long length)
{
  if(!m_XMLParser)
    {
    return false;
    }
  return m_XMLParser->Parse(buffer,length);
}

//--------------------------------------------------------------------------------------------------
std::string RestAPI::GetSessionId()
  {
  

  curl_slist* pCookies = 0; 
  curl_slist* p = 0; 
  
  std::string sessionid(""); 

  if (m_cURL != NULL)
    {
    // get list of cookies - let's assume the last one in the most recent
    //TODO: FIX this->m_CurlCode = curl_easy_getinfo( m_cURL, CURLINFO_COOKIELIST, &pCookies );
    if (this->m_CurlCode == CURLE_OK)
      {
      p = pCookies;
      while(p) 
        {
        //Logger::debug(p->data);
        std::vector<std::string> tokens;
        //TODO: FIX kwutils::tokenize(STR(p->data), tokens, "\t");
        if (tokens.size() == 7)
          {
          sessionid = tokens.at(6); 
          //Logger::debug("\tsession id:" + sessionid); 
          }
        p = p->next; 
        }
      if (pCookies != NULL)
        {
        curl_slist_free_all(pCookies);
        }
      }
    else 
      {
      //Logger::warning(STR("m_CurlCode:") + kwutils::to_string<m_CurlCode>(this->m_CurlCode) +
      //                STR(", ERROR:") + curl_easy_strerror(this->m_CurlCode));
      }
    }
  
  return sessionid; 
  }

//--------------------------------------------------------------------------------------------------
static size_t Curl_output_function(void *ptr, size_t size, size_t nitems, void *userp)
{
  char *buffer = (char*)ptr;
  RestAPI * restAPI = (RestAPI*)userp; 

  size_t length = size*nitems;   
  switch (restAPI->GetOutputMode())
    {
    case RestAPI::FILE:
      restAPI->GetOutputFileStream()->write(buffer, length);
      break; 
    case RestAPI::BUFFER:
      restAPI->GetOutputBuffer().append(buffer, length);
      restAPI->Parse(buffer,length);
      break;
    }
  return length; 
}

//--------------------------------------------------------------------------------------------------
static size_t Curl_read_function(void *bufptr, size_t size, size_t nitems, void *userp)
{
  char *buffer = (char*)bufptr;
  RestAPI * restAPI = (RestAPI*)userp; 
  size_t length = size*nitems;
  restAPI->GetInputStream()->read(buffer, length);
  return restAPI->GetInputStream()->gcount();
}


//-----------------------------------------------------------------------------
std::string RestAPI::SearchKeywords( const std::string &keywords)
  {
  // Format the search string
  // TODO: take care of extra '/' in the server URL
  const std::string searchURL = "api/rest/midas.item.search/?term=" +
                                this->FormatKeywords(keywords);

  // Send the request and download the .xml answer file
  std::string out;
  this->Download(out, searchURL, BUFFER);

  return out;
  }

//-----------------------------------------------------------------------------
std::string RestAPI::FormatKeywords(const std::string &keywords)
  {
  // Copy input string for modifications
  std::string out = keywords;
  /*
  // Replace weird characters by white space
  unsigned int i=0;
  unsigned int N=out.length();
  while(i++<N)
    {
    char c = out[i];
    if(c>'Z' && c<'A' && c>'z' && c<'a' && c<'0' && c>'9' && c!='+' && c!='-' && c!=' ')
      out[i] = ' ';
    }

  // Delete multiples spaces
  size_t pos;
  while((pos = out.find("  ")) != std::string::npos) // Look for double white spaces
    out.erase(pos, 1);

  // Delete first or last character space or + or - sign
  while(out[0]==' ' || IS_SEARCH_OPERATOR(out[0]))
    out.erase(0, 1);
  while(out[out.length()-1]==' ' || IS_SEARCH_OPERATOR(out[out.length()-1]))
    out.erase(out.length()-1, 1);

  //return out;
  // Replace '+' by "+AND+"
  // Replace '-' by "+NOT+"
  // Replace ' ' by "+OR+"
  i = 0;
  while(i++ < out.length())
    {
    char c = out[i];
    switch(c)
      {
      case '+':
        REMOVE_SURROUNDING_SPACE(out, i);
        out.replace(i,1, "+AND+");
        i+=4;
        break;
      case '-':
        REMOVE_SURROUNDING_SPACE(out, i);
        out.replace(i,1, "+NOT+");
        i+=4;
        break;
      case ' ':
        if(i+1<out.length() && IS_SEARCH_OPERATOR(out[i+1]))
          break;
        out.replace(i,1, "+OR+");
        i+=3;
        break;
      default:
        break;
      }
    }
*/
  
  return out;
  }
  
} // end namespace
