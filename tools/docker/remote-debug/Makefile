IMAGE="valkyrie00/cachegrand:remote-debug"

.PHONY: build
build:
	@docker build --force-rm --no-cache . -t $(IMAGE)

.PHONY: build-multiarch
build-multiarch:
	@docker buildx build --no-cache --push --platform linux/amd64,linux/arm64 -t $(IMAGE) .
