require 'rubygems'
require 'snmp'
require 'logger'

log = Logger.new('traps.log')
m = SNMP::TrapListener.new(:Port => 1062, :Community => 'public') do |manager|
    manager.on_trap_default do |trap|
        log.info trap.inspect
    end
end
m.join