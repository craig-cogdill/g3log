/** ==========================================================================
 * 2012 by KjellKod.cc. This is PUBLIC DOMAIN to use at your own risk and comes
 * with no warranties. This code is yours to share, use and modify with no
 * strings attached and no restrictions or obligations.
 * ============================================================================
 * Filename:g2logmessage.cpp  Part of Framework for Logging and Design By Contract
 * Created: 2012 by Kjell Hedström
 *
 * PUBLIC DOMAIN and Not copywrited. First published at KjellKod.cc
 * ********************************************* */

#include "g2logmessage.hpp"
#include "g2time.hpp"
#include "crashhandler.hpp"
#include <stdexcept> // exceptions
#include "g2log.hpp"
#include "g2log.ipp"

namespace g2 {


   std::string LogMessage::line() const {
      return std::to_string(_pimpl->_line);
   }


   std::string LogMessage::file() const {
      return _pimpl->_file;
   }


   std::string LogMessage::function() const {
      return _pimpl->_function;
   }


   std::string LogMessage::level() const {
      return _pimpl->_level.text;
   }


   std::string LogMessage::timestamp(const std::string & time_look) const {
      std::ostringstream oss;
      oss << localtime_formatted(_pimpl->_timestamp, time_look);
      return oss.str();
   }


   std::string LogMessage::microseconds() const {
      return std::to_string(_pimpl->_microseconds);
   }


   std::string LogMessage::message() const {
      return _pimpl->_stream.str();
   }


   std::string LogMessage::expression() const {
      return _pimpl->_expression;
   }


   bool LogMessage::wasFatal() const {
      return internal::wasFatal(_pimpl->_level);
   }

   // YYYY/MM/DD HH:MM:SS -- ref g2time.hpp/cpp
   //out << "\n" << localtime_formatted(system_time, date_formatted);
   //out << " " << localtime_formatted(system_time, time_formatted); // TODO: time kommer från LogEntry
   //out << "." << std::chrono::duration_cast<std::chrono::microseconds>(steady_time - _steady_start_time).count();
   //out << "\t" << message << std::flush;


   std::string LogMessage::toString() const {
      std::ostringstream oss;
      oss << "\n" << timestamp() << "." << microseconds() << "\t";

      
      oss << level() << " [" << file();
      oss << " L: " << line() << "]\t";

      // Non-fatal Log Message
      if (false == wasFatal()) {
         oss << '"' << message() << '"';
         return oss.str();
      }

      if (internal::FATAL_SIGNAL.value == _pimpl->_level.value) {
         oss << "\n\n***** FATAL SIGNAL RECEIVED ******* " << std::endl;
         oss << '"' << message() << '"';
         return oss.str();
      }


      // Not crash scenario but LOG or CONTRACT
      auto level_value = _pimpl->_level.value;
      if (FATAL.value == level_value) {
         oss << "\n\t*******\tEXIT trigger caused LOG(FATAL) entry: \n\t";
         oss << '"' << message() << '"';
         oss << "\n*******\tSTACKDUMP *******\n" << internal::stackdump();
      } else if (internal::CONTRACT.value == level_value) {
         oss << "\n\t  *******\tEXIT trigger caused by broken Contract: CHECK(" << _pimpl->_expression << ")\n\t";
         oss << '"' << message() << '"';
         oss << "\n*******\tSTACKDUMP *******\n" << internal::stackdump();
      } else {
         oss << "\n\t*******\tUNKNOWN Log Message Type\n" << '"' << message() << '"';
      }

      return oss.str();
   }


   std::ostringstream& LogMessage::stream() {
      return _pimpl->_stream;
   }


   LogMessage(std::shared_ptr<LogMessageImpl> details)
   : _pimpl(details) { }



   namespace internal {


      FatalMessage::FatalMessage(const std::string& crash_message, int signal_id)
      : FatalMessage({std::make_shared<LogMessageImpl>(crash_message)}, signal_id) { }


      FatalMessage(const LogMessage& message, int signal_id)
      : _crash_message(message), signal_id_(signal_id) { }


      FatalTrigger::FatalTrigger(const FatalMessage& exit_message) : _fatal_message(exit_message) { }


      FatalTrigger::~FatalTrigger() {
         // At destruction, flushes fatal message to g2LogWorker
         // either we will stay here until the background worker has received the fatal
         // message, flushed the crash message to the sinks and exits with the same fatal signal
         //..... OR it's in unit-test mode then we throw a std::runtime_error (and never hit sleep)
         fatalCall(_fatal_message);
      }
   } // internal
} // g2