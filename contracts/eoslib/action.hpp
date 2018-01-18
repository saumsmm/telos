/**
 *  @file
 *  @copyright defined in eos/LICENSE.txt
 */
#pragma once
#include <eoslib/action.h>
#include <eoslib/raw.hpp>

namespace eosio {

   /**
    * @defgroup actioncppapi Action C++ API
    * @ingroup actionapi
    * @brief Type-safe C++ wrapers for Action C API
    *
    * @note There are some methods from the @ref actioncapi that can be used directly from C++
    *
    * @{
    */

   /**
    *
    *  This method attempts to reinterpret the action body as type T. This will only work
    *  if the action has no dynamic fields and the struct packing on type T is properly defined.
    *
    *  @brief Interpret the action body as type T
    *  
    *  Example:
    *  @code
    *  struct dummy_action {
    *    char a; //1
    *    unsigned long long b; //8
    *    int  c; //4
    *  };
    *  dummy_action msg = current_action<dummy_action>();
    *  @endcode
    */
   template<typename T>
   T current_action() {
      T value;
      auto read = read_action( &value, sizeof(value) );
      assert( read >= sizeof(value), "action shorter than expected" );
      return value;
   }

   template<typename T>
   T unpack_action() {
      char buffer[action_size()];
      read_action( buffer, sizeof(buffer) );
      return raw::unpack<T>( buffer, sizeof(buffer) );
   }

   using ::require_auth;
   using ::require_recipient;

   /**
    *  All of the listed accounts will be added to the set of accounts to be notified
    *
    *  This helper method enables you to add multiple accounts to accounts to be notified list with a single
    *  call rather than having to call the similar C API multiple times.
    *
    *  @note action.code is also considered as part of the set of notified accounts
    *
    *  @brief Verify specified accounts exist in the set of notified accounts
    *
    *  Example:
    *  @code
    *  require_recipient(N(Account1), N(Account2), N(Account3)); // throws exception if any of them not in set.
    *  @endcode
    */
   template<typename... accounts>
   void require_recipient( account_name name, accounts... remaining_accounts ){
      require_recipient( name );
      require_recipient( remaining_accounts... );
   }

   struct permission_level {
      account_name    actor;
      permission_name permission;
      template<typename DataStream>

      friend DataStream& operator << ( DataStream& ds, const permission_level& a ){
         return ds << a.actor << a.permission;
      }

      template<typename DataStream>
      friend DataStream& operator >> ( DataStream& ds,  permission_level& a ){
         return ds >> a.actor >> a.permission;

      }
   };

   /**
    * This is the packed representation of an action along with
    * meta-data about the authorization levels.
    */
   struct action {
      account_name               account;
      action_name                name;
      vector<permission_level>   authorization;
      bytes                      data;

      action() = default;

      /**
       *  @tparam Action - a type derived from action_meta<Scope,Name>
       *  @param value - will be serialized via raw::pack into data
       */
      template<typename Action>
      action( vector<permission_level>&& auth, const Action& value ) {
         account       = Action::get_account();
         name          = Action::get_name();
         authorization = move(auth);
         data          = raw::pack(value);
      }

      template<typename DataStream>
      friend DataStream& operator << ( DataStream& ds, const action& a ){
         ds << a.account << a.name;
         raw::pack(ds, a.authorization);
         raw::pack(ds, a.data);
         return ds;
      }

      template<typename DataStream>
      friend DataStream& operator >> ( DataStream& ds,  action& a ){
         ds >> a.account >> a.name;
         raw::unpack(ds, a.authorization);
         raw::unpack(ds, a.data);
         return ds;
      }

      void send() const {
         auto serialize = raw::pack(*this);
         ::send_inline(serialize.data(), serialize.size());
      }
   };

   template<uint64_t Account, uint64_t Name>
   struct action_meta {
      static uint64_t get_account() { return Account; }
      static uint64_t get_name()  { return Name; }
   };


 ///@} actioncpp api

} // namespace eosio

EOSLIB_REFLECT( eosio::permission_level, (actor)(permission) )
EOSLIB_REFLECT( eosio::action, (account)(name)(authorization)(data) )