def print_usage_and_exit
  puts 'Usage: "ruby dvfs.rb <console|file> [<filename>]"'
  exit
end

print_usage_and_exit if ARGV.count < 1 || ARGV.first == 'file' && ARGV.count == 1

MODE_CONSOLE = ARGV.first == 'console'
MODE_FILE = ARGV.first != 'console'
FILENAME = ARGV.last

require 'csv'

SLACK = 4.0
QUANTUM_SIZE = SLACK / 10.0

# This is problem-agnostic part

class Task
  attr_reader :duration

  def initialize(duration, dissipation, voltage)
    @duration = duration
    @dissipation = dissipation
    @voltage = voltage
  end

  def extension
    (@duration + QUANTUM_SIZE) /  @duration
  end

  def energy
    @duration * @dissipation
  end

  def adjusted_energy
    (@dissipation * adjusted_voltage**2) / (self.extension * @voltage[:max]**2) * (@duration + QUANTUM_SIZE)
  end

  def extend_task!
    adjusted_energy = self.adjusted_energy

    @duration += QUANTUM_SIZE
    @dissipation = adjusted_energy / @duration
  end

private
  def adjusted_voltage
    extended_voltage = @voltage[:treshold] + initial_voltage / (2 * self.extension)
    extended_voltage + Math.sqrt(extended_voltage**2 - @voltage[:treshold]**2)
  end

  def initial_voltage
    (@voltage[:max] - @voltage[:treshold])**2 / @voltage[:max]
  end
end

# This is problem-specific part

TASKS = [
  Task.new(4, 3, { max: 3.3, treshold: 0.8 }),
  Task.new(2, 5, { max: 3.3, treshold: 0.8 }),
  Task.new(8, 3, { max: 2.8, treshold: 0.6 }),
  Task.new(5, 3, { max: 3.3, treshold: 0.8 }),
]

def path_a_c_fine
  (TASKS[0].duration + TASKS[2].duration + TASKS[3].duration) <= 21
end

def path_a_b_fine
  (TASKS[0].duration + TASKS[1].duration + TASKS[3].duration) <= 21
end

csv = []
while path_a_c_fine || path_a_b_fine
  tasks = TASKS.map do |task|
    task.energy - task.adjusted_energy
  end

  index = tasks.index(tasks.max)
  index = 1 if !path_a_c_fine && [0, 2, 3].include?(index)

  TASKS[index].extend_task!

  if MODE_CONSOLE
    puts tasks.map{ |t| sprintf '%0.03f', t.round(3) }.inspect + "\t" + (TASKS[0].duration + TASKS[2].duration + TASKS[3].duration).round(2).to_s + "\t" +(TASKS[0].duration + TASKS[1].duration + TASKS[3].duration).round(2).to_s + "\t" + ['A', 'B', 'C', 'D'][index].to_s
  else
    csv << tasks.push(['A', 'B', 'C', 'D'][index])
  end
end

if MODE_FILE
  CSV.open(FILENAME, 'wb') do |csv_file|
    csv.each{ |line| csv_file << line }
  end
end
