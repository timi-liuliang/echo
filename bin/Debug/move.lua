local Move = {
	extends=Live2DCubism,
	posX = 100,
}

function Move:start()
	log:info("start node live2dcubism")
end

function Move:update()
	if self.posX > 500 then
		self.posX = 0
	end

	self.posX = self.posX + 1
	self:setPosX(self.posX)
end

return Move