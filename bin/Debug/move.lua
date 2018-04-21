local Move = {
	extends=Live2DCubism,
	posX = 0,
}

function Move:start()
	log:info("start node live2dcubism")
end

function Move:update()
	if self.posX < -250 then
		self.posX = 0
	end

	self.posX = self.posX - 0.3
	self:setPosX(self.posX)
	
	print = nil
	print(self.posX)
end

return Move