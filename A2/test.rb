require 'json'

class Processor
	def initialize(p)
		@port = p
		@count = 0
		@cycles, @cpu, @total = 0, 0, 0
	end

	def process(v)
		result = JSON.parse(`./lab2 -u -p #{@port} -j -V "\\"#{v}\\""`)

		@cycles += result['cycles']
		@cpu += result['time']['cpu']
		@total += result['time']['total']

		@count += 1
	end

	def result
		[@cycles, @cpu, @total].map { |v| v / @count.to_f }
	end
end

if ARGV.length < 1
	puts "usage: ./#{__FILE__} port [tries=100]"
else
	port = ARGV[0].to_i
	tries = (ARGV[1] || 100).to_i

	while (line = STDIN.gets)
		line.strip!

		next if line.empty?

		uut = Processor.new(port)
		i = 0

		tries.times do
			STDOUT.write("#{line}: [#{i}/#{tries}]\r")
			i = uut.process(line)
		end

		cycles, cpu, total = *uut.result.map { |v| v.round(1) }
		puts "#{line}: #{cycles} cycles, #{cpu}s CPU time and #{total}s total"
	end
end

