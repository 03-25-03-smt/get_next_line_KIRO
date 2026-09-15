/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   get_next_line_bonus.c                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vladyslb <vladyslb@student.42prague.com>   +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/15 09:00:00 by vladyslb          #+#    #+#             */
/*   Updated: 2026/09/15 09:00:00 by vladyslb         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "get_next_line_bonus.h"

static char	*get_line(char **buffer)
{
	char	*line;
	char	*keep;
	size_t	til_null_len;
	size_t	til_new;

	if (!buffer || !*buffer)
		return (NULL);
	til_new = strlen_at(*buffer, '\n');
	if ((*buffer)[til_new] == '\n')
		til_new++;
	line = cpy_buffer(*buffer, 0, til_new);
	if (!line)
		return (NULL);
	til_null_len = strlen_at(*buffer, '\0');
	keep = cpy_buffer(*buffer, til_new, til_null_len - til_new);
	if (!keep)
		return (free(line), NULL);
	free(*buffer);
	*buffer = keep;
	return (line);
}

static char	*get_current_buffer(int fd, char *buffer)
{
	char	*current;
	ssize_t	bytes_read;

	bytes_read = 1;
	current = (char *)malloc(BUFFER_SIZE + 1);
	if (!current)
		return (NULL);
	while (bytes_read > 0 && !find_chr(buffer, '\n'))
	{
		bytes_read = read(fd, current, BUFFER_SIZE);
		if (bytes_read == 0)
			break ;
		if (bytes_read == -1)
			return (free(current), free(buffer), NULL);
		current[bytes_read] = '\0';
		buffer = merge_previous_and_current(buffer, current);
		if (!buffer)
			return (free(current), NULL);
	}
	free(current);
	if (strlen_at(buffer, '\0') > 0)
		return (buffer);
	return (NULL);
}

char	*get_next_line(int fd)
{
	static char	*buffer[MAX_FILES];
	char		*line;
	char		test;

	if (fd < 0 || BUFFER_SIZE <= 0 || fd >= MAX_FILES)
		return (NULL);
	if (read(fd, &test, 0) == -1)
		return (free(buffer[fd]), buffer[fd] = NULL, NULL);
	buffer[fd] = get_current_buffer(fd, buffer[fd]);
	if (!buffer[fd])
		return (NULL);
	line = get_line(&buffer[fd]);
	if (!line)
		return (free(buffer[fd]), buffer[fd] = NULL, NULL);
	if (!buffer[fd][0])
	{
		free(buffer[fd]);
		buffer[fd] = NULL;
	}
	return (line);
}
